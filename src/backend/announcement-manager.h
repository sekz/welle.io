/*
 *    Copyright (C) 2025
 *    welle.io Thailand DAB+ Receiver
 *
 *    This file is part of the welle.io.
 *    Many of the ideas as implemented in welle.io are derived from
 *    other work, made available through the GNU general Public License.
 *    All copyrights of the original authors are recognized.
 *
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

#ifndef ANNOUNCEMENT_MANAGER_H
#define ANNOUNCEMENT_MANAGER_H

#include "announcement-types.h"
#include <cstdint>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>

/**
 * @file announcement-manager.h
 * @brief DAB Announcement Switching Manager (ETSI EN 300 401)
 *
 * Manages automatic switching between regular service playback and announcements
 * based on FIG 0/18 (announcement support) and FIG 0/19 (active announcements).
 *
 * ETSI EN 300 401 References:
 * - Section 8.1.6.1: FIG 0/18 Announcement support
 * - Section 8.1.6.2: FIG 0/19 Announcement switching
 * - Section 5.2.2.1: Announcement types (ASu flags, ASw flags)
 *
 * Architecture:
 * - State machine (6 states) manages announcement lifecycle
 * - User preferences control switching behavior (type filters, priority threshold)
 * - Thread-safe: all operations protected by mutex
 * - Integrates with RadioController for service switching coordination
 */

/**
 * @struct AnnouncementPreferences
 * @brief User preferences for announcement switching behavior
 *
 * Controls which announcements trigger automatic switching and how the
 * receiver behaves during announcements.
 */
struct AnnouncementPreferences {
    bool enabled = true;  ///< Master enable/disable for announcement switching

    // Per-type enable/disable (map: AnnouncementType → enabled)
    // If a type is not in the map, it defaults to enabled
    std::unordered_map<AnnouncementType, bool> type_enabled;

    // Priority threshold: only switch to announcements with priority <= threshold
    // Priority: 1 (highest, Alarm) to 11 (lowest, Finance)
    // Default: 1 means all priorities accepted (since 1 is highest)
    int priority_threshold = 11;

    // Allow manual return to original service before announcement ends
    // If true, user can press "Return" button during announcement
    // If false, must wait for announcement to end (ASw = 0x0000)
    bool allow_manual_return = true;

    // Maximum announcement duration (safety timeout)
    // If announcement exceeds this, auto-return to original service
    // Prevents being stuck in announcement if ASw never goes to 0x0000
    std::chrono::seconds max_announcement_duration{300};  // 5 minutes default

    // Default constructor initializes all types as enabled
    AnnouncementPreferences() {
        for (uint8_t i = 0; i <= static_cast<uint8_t>(AnnouncementType::MAX_TYPE); ++i) {
            type_enabled[static_cast<AnnouncementType>(i)] = true;
        }
    }
};

/**
 * @class AnnouncementManager
 * @brief Manages DAB announcement switching lifecycle
 *
 * Responsibilities:
 * 1. Store announcement support data from FIG 0/18
 * 2. Process active announcements from FIG 0/19
 * 3. Apply user preferences and priority filtering
 * 4. Maintain state machine for switching
 * 5. Coordinate with RadioController for service switching
 * 6. Track announcement history for statistics
 *
 * State Machine (ETSI EN 300 401 Section 8.1.6.3):
 *
 *   Idle ──────────────────────────┐
 *    ↑                              ↓
 *    │                      AnnouncementDetected
 *    │                              ↓
 *    │                   SwitchingToAnnouncement
 *    │                              ↓
 *    │                     PlayingAnnouncement
 *    │                              ↓
 *    │                      AnnouncementEnding
 *    │                              ↓
 *    └────────────  RestoringOriginalService
 *
 * Thread Safety: All public methods are thread-safe (mutex-protected)
 *
 * Usage Example:
 * @code
 *   AnnouncementManager manager;
 *
 *   // Set user preferences
 *   AnnouncementPreferences prefs;
 *   prefs.priority_threshold = 3;  // Only Alarm, Traffic, Transport
 *   manager.setUserPreferences(prefs);
 *
 *   // FIBProcessor calls this when FIG 0/18 received
 *   ServiceAnnouncementSupport support;
 *   support.service_id = 0xC221;
 *   support.support_flags.set(AnnouncementType::Alarm);
 *   support.support_flags.set(AnnouncementType::RoadTraffic);
 *   manager.updateAnnouncementSupport(support);
 *
 *   // FIBProcessor calls this when FIG 0/19 received
 *   std::vector<ActiveAnnouncement> announcements;
 *   ActiveAnnouncement ann;
 *   ann.active_flags.set(AnnouncementType::RoadTraffic);
 *   ann.subchannel_id = 5;
 *   announcements.push_back(ann);
 *   manager.updateActiveAnnouncements(announcements);
 *
 *   // Check if should switch
 *   if (manager.shouldSwitchToAnnouncement(ann)) {
 *       manager.switchToAnnouncement(ann);
 *       // RadioController switches to subchannel 5
 *   }
 * @endcode
 */
class AnnouncementManager {
public:
    AnnouncementManager();
    ~AnnouncementManager() = default;

    // Prevent copying (contains mutex)
    AnnouncementManager(const AnnouncementManager&) = delete;
    AnnouncementManager& operator=(const AnnouncementManager&) = delete;

    // ========================================================================
    // Configuration Methods
    // ========================================================================

    /**
     * @brief Set user preferences for announcement switching
     * @param prefs User preferences structure
     *
     * Thread-safe. Updates internal preferences and applies to future
     * announcement switching decisions.
     */
    void setUserPreferences(const AnnouncementPreferences& prefs);

    /**
     * @brief Set ensemble alarm flag (Al flag from FIG 0/0)
     * @param enabled true if alarm announcements enabled, false otherwise
     *
     * Per ETSI EN 300 401 Section 8.1.2:
     * - Al=1: Alarm announcements (cluster 0xFF) MUST be switched to
     * - Al=0: Alarm announcements (cluster 0xFF) MUST be ignored
     *
     * This overrides all user preferences for cluster 0xFF announcements.
     * Thread-safe.
     */
    void setEnsembleAlarmEnabled(bool enabled);

    /**
     * @brief Enable/disable specific announcement type
     * @param type Announcement type
     * @param enable true to enable, false to disable
     *
     * Thread-safe. Convenience method to update single type preference.
     */
    void enableAnnouncementType(AnnouncementType type, bool enable);

    /**
     * @brief Get current user preferences
     * @return Copy of current preferences
     *
     * Thread-safe. Returns a copy to avoid race conditions.
     */
    AnnouncementPreferences getUserPreferences() const;

    // ========================================================================
    // Announcement Data Updates (called by FIBProcessor)
    // ========================================================================

    /**
     * @brief Update announcement support for a service (FIG 0/18)
     * @param support Service announcement support data
     *
     * Called when FIG 0/18 is decoded. Stores which announcement types
     * the service supports. This data is used to validate active announcements.
     *
     * Thread-safe.
     */
    void updateAnnouncementSupport(const ServiceAnnouncementSupport& support);

    /**
     * @brief Update active announcements (FIG 0/19)
     * @param announcements Vector of active announcements
     *
     * Called when FIG 0/19 is decoded. Triggers switching logic if
     * announcements meet user preferences.
     *
     * Key behaviors:
     * - If ASw = 0x0000, announcement has ended → return to original service
     * - If ASw != 0x0000 and meets preferences → switch to announcement
     * - If already in higher priority announcement → ignore lower priority
     *
     * Thread-safe.
     */
    void updateActiveAnnouncements(const std::vector<ActiveAnnouncement>& announcements);

    /**
     * @brief Clear announcement support for a service
     * @param service_id Service ID to clear
     *
     * Called when service is removed or ensemble changes.
     * Thread-safe.
     */
    void clearAnnouncementSupport(uint32_t service_id);

    /**
     * @brief Clear all announcement data
     *
     * Called on ensemble change or receiver reset.
     * Resets to Idle state if in announcement.
     * Thread-safe.
     */
    void clearAllData();

    // ========================================================================
    // Switching Logic
    // ========================================================================

    /**
     * @brief Check if should switch to announcement
     * @param ann Active announcement to evaluate
     * @return true if should switch, false otherwise
     *
     * Decision criteria (ETSI EN 300 401 Section 8.1.6.2):
     * 1. Feature enabled? (prefs.enabled)
     * 2. Already in higher/equal priority announcement? (compare priorities)
     * 3. Announcement type enabled? (prefs.type_enabled)
     * 4. Priority threshold met? (priority <= prefs.priority_threshold)
     * 5. Current service supports this type? (check FIG 0/18 data)
     *
     * Thread-safe.
     */
    bool shouldSwitchToAnnouncement(const ActiveAnnouncement& ann) const;

    /**
     * @brief Initiate switch to announcement
     * @param ann Active announcement to switch to
     *
     * State transition: Idle → AnnouncementDetected → SwitchingToAnnouncement
     *
     * Saves current service context (service ID, subchannel ID) and
     * transitions state. RadioController should then call setService()
     * with announcement subchannel.
     *
     * Thread-safe.
     */
    void switchToAnnouncement(const ActiveAnnouncement& ann);

    /**
     * @brief Confirm announcement playback started
     *
     * State transition: SwitchingToAnnouncement → PlayingAnnouncement
     *
     * Called by RadioController after successfully tuning to announcement
     * subchannel. Starts announcement timer for timeout monitoring.
     *
     * Thread-safe.
     */
    void confirmAnnouncementStarted();

    /**
     * @brief Return to original service
     *
     * State transition: PlayingAnnouncement → AnnouncementEnding → RestoringOriginalService → Idle
     *
     * Restores original service playback. Can be called:
     * 1. Automatically when announcement ends (ASw = 0x0000)
     * 2. Manually by user (if allow_manual_return = true)
     * 3. On timeout (max_announcement_duration exceeded)
     *
     * Adds announcement to history for statistics.
     *
     * Thread-safe.
     */
    void returnToOriginalService();

    /**
     * @brief Set original service context (before first announcement)
     * @param service_id Service ID
     * @param subchannel_id Subchannel ID
     *
     * Should be called by RadioController when starting service playback.
     * This is the service to restore after announcement ends.
     *
     * Thread-safe.
     */
    void setOriginalService(uint32_t service_id, uint8_t subchannel_id);

    // ========================================================================
    // State Queries
    // ========================================================================

    /**
     * @brief Get current state
     * @return Current announcement state
     *
     * Thread-safe.
     */
    AnnouncementState getState() const;

    /**
     * @brief Check if currently in announcement
     * @return true if state is PlayingAnnouncement, false otherwise
     *
     * Thread-safe. Convenience method for RadioController.
     */
    bool isInAnnouncement() const;

    /**
     * @brief Get current announcement (if active)
     * @return Current announcement, or default-constructed if not in announcement
     *
     * Thread-safe. Use isInAnnouncement() to check validity.
     */
    ActiveAnnouncement getCurrentAnnouncement() const;

    /**
     * @brief Get announcement duration (if active)
     * @return Duration in seconds, or 0 if not in announcement
     *
     * Thread-safe. Returns time since switchToAnnouncement() was called.
     */
    int getAnnouncementDuration() const;

    /**
     * @brief Get original service ID (saved before announcement)
     * @return Original service ID, or 0 if not in announcement
     *
     * Thread-safe.
     */
    uint32_t getOriginalServiceId() const;

    /**
     * @brief Get original subchannel ID (saved before announcement)
     * @return Original subchannel ID, or 0 if not in announcement
     *
     * Thread-safe.
     */
    uint8_t getOriginalSubchannelId() const;

    // ========================================================================
    // History Management
    // ========================================================================

    /**
     * @brief Add completed announcement to history
     * @param ann Announcement that ended
     * @param duration Announcement duration
     *
     * Called internally when announcement ends. Used for statistics.
     * History is limited to last N entries to prevent unbounded growth.
     *
     * Thread-safe.
     */
    void addToHistory(const ActiveAnnouncement& ann, std::chrono::seconds duration);

private:
    // ========================================================================
    // State Management
    // ========================================================================

    AnnouncementState state_;                  ///< Current state machine state
    AnnouncementPreferences prefs_;            ///< User preferences
    bool ensemble_alarm_enabled_;              ///< Al flag from FIG 0/0 (ensemble alarm enabled)

    // Current context (saved during announcement)
    uint32_t original_service_id_;             ///< Service ID before announcement
    uint8_t original_subchannel_id_;           ///< Subchannel ID before announcement
    ActiveAnnouncement current_announcement_;  ///< Current active announcement
    std::chrono::steady_clock::time_point announcement_start_time_;  ///< When switched to announcement

    // ========================================================================
    // Data Storage (from FIG 0/18 and FIG 0/19)
    // ========================================================================

    // Service announcement support (FIG 0/18)
    // Map: service_id → ServiceAnnouncementSupport
    std::unordered_map<uint32_t, ServiceAnnouncementSupport> service_support_;

    // Active announcements (FIG 0/19)
    // Map: cluster_id → ActiveAnnouncement
    std::unordered_map<uint8_t, ActiveAnnouncement> active_announcements_;

    // ========================================================================
    // Thread Safety
    // ========================================================================

    mutable std::mutex mutex_;  ///< Protects all shared data

    // ========================================================================
    // Helper Methods
    // ========================================================================

    /**
     * @brief Check if announcement type is enabled in preferences
     * @param type Announcement type
     * @return true if enabled, false otherwise
     */
    bool isAnnouncementTypeEnabled(AnnouncementType type) const;

    /**
     * @brief Transition to new state
     * @param new_state Target state
     *
     * Updates state_ and logs transition (debug builds).
     * Must be called with mutex_ held.
     */
    void transitionState(AnnouncementState new_state);

    /**
     * @brief Check if announcement timeout exceeded
     * @return true if timeout exceeded, false otherwise
     *
     * Must be called with mutex_ held.
     */
    bool isAnnouncementTimeoutExceeded() const;

    /**
     * @brief Get state name for logging
     * @param state State to convert
     * @return State name string
     */
    const char* getStateName(AnnouncementState state) const;
};

#endif // ANNOUNCEMENT_MANAGER_H
