/*
 *    Copyright (C) 2025
 *    welle.io Thailand DAB+ Receiver
 *
 *    This file is part of the welle.io.
 *    Many of the ideas as implemented in welle.io are derived from
 *    other work, made available through the GNU general Public License.
 *    All copyrights of the original authors are recognized.
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    welle.io is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with welle.io; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "announcement-manager.h"
#include <algorithm>
#include <iostream>
#include <cstring>

/**
 * @file announcement-manager.cpp
 * @brief Implementation of DAB Announcement Switching Manager
 *
 * Implements state machine and switching logic for DAB announcements
 * according to ETSI EN 300 401 Section 8.1.6.
 */

// ============================================================================
// Constructor / Destructor
// ============================================================================

AnnouncementManager::AnnouncementManager()
    : state_(AnnouncementState::Idle)
    , original_service_id_(0)
    , original_subchannel_id_(0)
{
    // Initialize with default preferences (all types enabled)
    prefs_ = AnnouncementPreferences();
}

// ============================================================================
// Configuration Methods
// ============================================================================

void AnnouncementManager::setUserPreferences(const AnnouncementPreferences& prefs)
{
    std::lock_guard<std::mutex> lock(mutex_);

    prefs_ = prefs;

    // Validate priority threshold (1-11 range)
    if (prefs_.priority_threshold < 1) {
        prefs_.priority_threshold = 1;
    }
    if (prefs_.priority_threshold > 11) {
        prefs_.priority_threshold = 11;
    }

    // Log configuration change
    #ifdef DEBUG_ANNOUNCEMENT
    std::clog << "AnnouncementManager: Preferences updated - enabled="
              << (prefs_.enabled ? "true" : "false")
              << " priority_threshold=" << prefs_.priority_threshold
              << " allow_manual_return=" << (prefs_.allow_manual_return ? "true" : "false")
              << " max_duration=" << prefs_.max_announcement_duration.count() << "s"
              << std::endl;
    #endif
}

void AnnouncementManager::enableAnnouncementType(AnnouncementType type, bool enable)
{
    std::lock_guard<std::mutex> lock(mutex_);

    prefs_.type_enabled[type] = enable;

    #ifdef DEBUG_ANNOUNCEMENT
    std::clog << "AnnouncementManager: Type "
              << getAnnouncementTypeName(type)
              << " " << (enable ? "enabled" : "disabled")
              << std::endl;
    #endif
}

AnnouncementPreferences AnnouncementManager::getUserPreferences() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return prefs_;  // Return copy
}

// ============================================================================
// Announcement Data Updates
// ============================================================================

void AnnouncementManager::updateAnnouncementSupport(const ServiceAnnouncementSupport& support)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Defensive: validate service ID
    if (support.service_id == 0) {
        #ifdef DEBUG_ANNOUNCEMENT
        std::cerr << "AnnouncementManager: Invalid service_id=0 in announcement support"
                  << std::endl;
        #endif
        return;
    }

    // Store or update announcement support
    service_support_[support.service_id] = support;

    #ifdef DEBUG_ANNOUNCEMENT
    std::clog << "AnnouncementManager: FIG 0/18 - Service 0x" << std::hex << support.service_id
              << std::dec << " supports " << support.support_flags.getActiveTypes().size()
              << " announcement types, " << support.cluster_ids.size() << " clusters"
              << std::endl;
    #endif
}

void AnnouncementManager::updateActiveAnnouncements(const std::vector<ActiveAnnouncement>& announcements)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Update active announcements map
    active_announcements_.clear();
    for (const auto& ann : announcements) {
        if (ann.isActive()) {
            active_announcements_[ann.cluster_id] = ann;

            #ifdef DEBUG_ANNOUNCEMENT
            std::clog << "AnnouncementManager: FIG 0/19 - Cluster " << static_cast<int>(ann.cluster_id)
                      << " active, type=" << getAnnouncementTypeName(ann.getHighestPriorityType())
                      << " subch=" << static_cast<int>(ann.subchannel_id)
                      << std::endl;
            #endif
        }
    }

    // Check if should switch to any announcement
    for (const auto& ann : announcements) {
        if (!ann.isActive()) {
            // Announcement ended (ASw = 0x0000)
            if (state_ == AnnouncementState::PlayingAnnouncement &&
                current_announcement_.cluster_id == ann.cluster_id) {
                // Current announcement ended - return to original service
                #ifdef DEBUG_ANNOUNCEMENT
                std::clog << "AnnouncementManager: Current announcement ended (ASw=0x0000), returning to service"
                          << std::endl;
                #endif
                returnToOriginalService();
            }
        }
        else if (state_ == AnnouncementState::Idle) {
            // Note: Switching is handled by RadioController calling handleAnnouncementStarted()
            // Don't duplicate state transitions here to avoid race conditions
            // if (shouldSwitchToAnnouncement(ann)) {
            //     #ifdef DEBUG_ANNOUNCEMENT
            //     std::clog << "AnnouncementManager: Auto-switching to announcement type="
            //               << getAnnouncementTypeName(ann.getHighestPriorityType())
            //               << " subch=" << static_cast<int>(ann.subchannel_id)
            //               << std::endl;
            //     #endif
            //     transitionState(AnnouncementState::AnnouncementDetected);
            // }
        }
    }

    // Check for timeout if in announcement
    if (state_ == AnnouncementState::PlayingAnnouncement) {
        if (isAnnouncementTimeoutExceeded()) {
            #ifdef DEBUG_ANNOUNCEMENT
            std::clog << "AnnouncementManager: Announcement timeout exceeded, returning to service"
                      << std::endl;
            #endif
            returnToOriginalService();
        }
    }
}

void AnnouncementManager::clearAnnouncementSupport(uint32_t service_id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    service_support_.erase(service_id);

    #ifdef DEBUG_ANNOUNCEMENT
    std::clog << "AnnouncementManager: Cleared announcement support for service 0x"
              << std::hex << service_id << std::dec << std::endl;
    #endif
}

void AnnouncementManager::clearAllData()
{
    std::lock_guard<std::mutex> lock(mutex_);

    service_support_.clear();
    active_announcements_.clear();

    // If in announcement, reset to idle
    if (state_ != AnnouncementState::Idle) {
        #ifdef DEBUG_ANNOUNCEMENT
        std::clog << "AnnouncementManager: Clearing all data, resetting to Idle state"
                  << std::endl;
        #endif
        transitionState(AnnouncementState::Idle);
        original_service_id_ = 0;
        original_subchannel_id_ = 0;
    }
}

// ============================================================================
// Switching Logic
// ============================================================================

bool AnnouncementManager::shouldSwitchToAnnouncement(const ActiveAnnouncement& ann) const
{
    // Note: Caller must hold mutex_ lock

    // 1. Feature enabled?
    if (!prefs_.enabled) {
        #ifdef DEBUG_ANNOUNCEMENT
        std::clog << "AnnouncementManager: Not switching - feature disabled" << std::endl;
        #endif
        return false;
    }

    // 2. Announcement active?
    if (!ann.isActive()) {
        #ifdef DEBUG_ANNOUNCEMENT
        std::clog << "AnnouncementManager: Not switching - announcement not active" << std::endl;
        #endif
        return false;
    }

    // 3. Already in announcement?
    if (state_ == AnnouncementState::PlayingAnnouncement) {
        // Check priority - only switch to higher priority
        AnnouncementType current_type = current_announcement_.getHighestPriorityType();
        AnnouncementType new_type = ann.getHighestPriorityType();
        int current_priority = getAnnouncementPriority(current_type);
        int new_priority = getAnnouncementPriority(new_type);

        if (new_priority >= current_priority) {
            // New announcement has equal or lower priority - don't switch
            #ifdef DEBUG_ANNOUNCEMENT
            std::clog << "AnnouncementManager: Not switching - already in higher/equal priority announcement"
                      << " (current=" << current_priority << " new=" << new_priority << ")"
                      << std::endl;
            #endif
            return false;
        }
    }

    // 4. Announcement type enabled?
    AnnouncementType ann_type = ann.getHighestPriorityType();
    if (!isAnnouncementTypeEnabled(ann_type)) {
        #ifdef DEBUG_ANNOUNCEMENT
        std::clog << "AnnouncementManager: Not switching - type "
                  << getAnnouncementTypeName(ann_type) << " disabled"
                  << std::endl;
        #endif
        return false;
    }

    // 5. Priority threshold met?
    int priority = getAnnouncementPriority(ann_type);
    if (priority > prefs_.priority_threshold) {
        #ifdef DEBUG_ANNOUNCEMENT
        std::clog << "AnnouncementManager: Not switching - priority " << priority
                  << " below threshold " << prefs_.priority_threshold
                  << std::endl;
        #endif
        return false;
    }

    // 6. Current service participates in announcement cluster?
    // (Only check if we have original service set)
    if (original_service_id_ != 0) {
        auto it = service_support_.find(original_service_id_);
        if (it != service_support_.end()) {
            // Check if current service participates in the announcement's cluster
            // (A service can receive announcements from a cluster without producing them)
            bool participates_in_cluster = std::find(
                it->second.cluster_ids.begin(),
                it->second.cluster_ids.end(),
                ann.cluster_id
            ) != it->second.cluster_ids.end();

            if (!participates_in_cluster) {
                #ifdef DEBUG_ANNOUNCEMENT
                std::clog << "AnnouncementManager: Not switching - current service 0x"
                          << std::hex << original_service_id_ << std::dec
                          << " doesn't participate in cluster "
                          << static_cast<int>(ann.cluster_id) << std::endl;
                #endif
                return false;
            }

            #ifdef DEBUG_ANNOUNCEMENT
            std::clog << "AnnouncementManager: Service 0x"
                      << std::hex << original_service_id_ << std::dec
                      << " participates in cluster "
                      << static_cast<int>(ann.cluster_id)
                      << " - switching allowed" << std::endl;
            #endif
        }
    }

    // All checks passed - should switch
    return true;
}

void AnnouncementManager::switchToAnnouncement(const ActiveAnnouncement& ann)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Defensive: validate announcement is active
    if (!ann.isActive()) {
        #ifdef DEBUG_ANNOUNCEMENT
        std::cerr << "AnnouncementManager: Cannot switch to inactive announcement"
                  << std::endl;
        #endif
        return;
    }

    // Save current announcement
    current_announcement_ = ann;
    announcement_start_time_ = std::chrono::steady_clock::now();

    // Transition state
    transitionState(AnnouncementState::SwitchingToAnnouncement);

    #ifdef DEBUG_ANNOUNCEMENT
    std::clog << "AnnouncementManager: Switching to announcement type="
              << getAnnouncementTypeName(ann.getHighestPriorityType())
              << " cluster=" << static_cast<int>(ann.cluster_id)
              << " subch=" << static_cast<int>(ann.subchannel_id)
              << std::endl;
    #endif
}

void AnnouncementManager::confirmAnnouncementStarted()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ == AnnouncementState::SwitchingToAnnouncement) {
        transitionState(AnnouncementState::PlayingAnnouncement);

        #ifdef DEBUG_ANNOUNCEMENT
        std::clog << "AnnouncementManager: Announcement playback confirmed"
                  << std::endl;
        #endif
    }
}

void AnnouncementManager::returnToOriginalService()
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Only return if in announcement
    if (state_ != AnnouncementState::PlayingAnnouncement &&
        state_ != AnnouncementState::SwitchingToAnnouncement) {
        #ifdef DEBUG_ANNOUNCEMENT
        std::clog << "AnnouncementManager: Not in announcement, ignoring return request"
                  << std::endl;
        #endif
        return;
    }

    // Calculate duration
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        now - announcement_start_time_);

    // Add to history
    addToHistory(current_announcement_, duration);

    // Transition through states
    transitionState(AnnouncementState::AnnouncementEnding);
    transitionState(AnnouncementState::RestoringOriginalService);

    #ifdef DEBUG_ANNOUNCEMENT
    std::clog << "AnnouncementManager: Returning to original service 0x"
              << std::hex << original_service_id_ << std::dec
              << " subch=" << static_cast<int>(original_subchannel_id_)
              << " duration=" << duration.count() << "s"
              << std::endl;
    #endif

    // Transition to idle
    transitionState(AnnouncementState::Idle);
}

void AnnouncementManager::setOriginalService(uint32_t service_id, uint8_t subchannel_id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Defensive: validate inputs
    if (service_id == 0) {
        #ifdef DEBUG_ANNOUNCEMENT
        std::cerr << "AnnouncementManager: Invalid service_id=0 in setOriginalService"
                  << std::endl;
        #endif
        return;
    }

    original_service_id_ = service_id;
    original_subchannel_id_ = subchannel_id;

    #ifdef DEBUG_ANNOUNCEMENT
    std::clog << "AnnouncementManager: Original service set to 0x"
              << std::hex << service_id << std::dec
              << " subch=" << static_cast<int>(subchannel_id)
              << std::endl;
    #endif
}

// ============================================================================
// State Queries
// ============================================================================

AnnouncementState AnnouncementManager::getState() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

bool AnnouncementManager::isInAnnouncement() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return state_ == AnnouncementState::PlayingAnnouncement;
}

ActiveAnnouncement AnnouncementManager::getCurrentAnnouncement() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return current_announcement_;  // Return copy
}

int AnnouncementManager::getAnnouncementDuration() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (state_ != AnnouncementState::PlayingAnnouncement) {
        return 0;
    }

    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        now - announcement_start_time_);

    return static_cast<int>(duration.count());
}

uint32_t AnnouncementManager::getOriginalServiceId() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return original_service_id_;
}

uint8_t AnnouncementManager::getOriginalSubchannelId() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return original_subchannel_id_;
}

// ============================================================================
// History Management
// ============================================================================

void AnnouncementManager::addToHistory(const ActiveAnnouncement& ann, std::chrono::seconds duration)
{
    // Note: Caller must hold mutex_ lock

    // Implementation note: History storage would be added here
    // For now, just log
    #ifdef DEBUG_ANNOUNCEMENT
    std::clog << "AnnouncementManager: History - type="
              << getAnnouncementTypeName(ann.getHighestPriorityType())
              << " duration=" << duration.count() << "s"
              << std::endl;
    #endif

    // TODO: Add to persistent history structure
    // (Could be std::deque<HistoryEntry> with max size limit)
}

// ============================================================================
// Helper Methods
// ============================================================================

bool AnnouncementManager::isAnnouncementTypeEnabled(AnnouncementType type) const
{
    // Note: Caller must hold mutex_ lock

    auto it = prefs_.type_enabled.find(type);
    if (it != prefs_.type_enabled.end()) {
        return it->second;
    }

    // Default to enabled if not in map
    return true;
}

void AnnouncementManager::transitionState(AnnouncementState new_state)
{
    // Note: Caller must hold mutex_ lock

    if (state_ == new_state) {
        return;  // No change
    }

    #ifdef DEBUG_ANNOUNCEMENT
    std::clog << "AnnouncementManager: State transition: "
              << getStateName(state_) << " → " << getStateName(new_state)
              << std::endl;
    #endif

    state_ = new_state;
}

bool AnnouncementManager::isAnnouncementTimeoutExceeded() const
{
    // Note: Caller must hold mutex_ lock

    if (state_ != AnnouncementState::PlayingAnnouncement) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - announcement_start_time_);

    return elapsed >= prefs_.max_announcement_duration;
}

const char* AnnouncementManager::getStateName(AnnouncementState state) const
{
    switch (state) {
        case AnnouncementState::Idle:
            return "Idle";
        case AnnouncementState::AnnouncementDetected:
            return "AnnouncementDetected";
        case AnnouncementState::SwitchingToAnnouncement:
            return "SwitchingToAnnouncement";
        case AnnouncementState::PlayingAnnouncement:
            return "PlayingAnnouncement";
        case AnnouncementState::AnnouncementEnding:
            return "AnnouncementEnding";
        case AnnouncementState::RestoringOriginalService:
            return "RestoringOriginalService";
        default:
            return "Unknown";
    }
}
