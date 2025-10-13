/*
 *    Copyright (C) 2017 - 2021
 *    Albrecht Lohofener (albrechtloh@gmx.de)
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../texts"

ComboBox {
    id: comboBox

    property bool sizeToContents
    property int modelWidth

    // Calculate width components explicitly for DPI-aware sizing
    readonly property int indicatorSpace: indicator ? indicator.width + spacing : 0
    readonly property int contentPadding: leftPadding + rightPadding
    readonly property int widthMargin: Units.dp(20)  // Scalable margin

    width: (sizeToContents) ? modelWidth + indicatorSpace + contentPadding + widthMargin : implicitWidth
    Layout.preferredWidth: width

    font.pixelSize: TextStyle.textStandartSize
    leftPadding: 12
    rightPadding: 12

    // Ensure text is visible in the ComboBox button
    contentItem: Text {
        leftPadding: 0
        rightPadding: comboBox.indicator.width + comboBox.spacing
        text: comboBox.displayText
        font: comboBox.font
        color: comboBox.enabled ? TextStyle.textColor : Qt.rgba(TextStyle.textColor.r, TextStyle.textColor.g, TextStyle.textColor.b, 0.3)
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    delegate: ItemDelegate {
        width: comboBox.width
        contentItem: TextStandart {
            text: modelData
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        highlighted: comboBox.highlightedIndex === index
    }

    Keys.onShortcutOverride: {
        if (event.key == Qt.Key_M || event.key == Qt.Key_S || event.key == Qt.Key_P || event.key == Qt.Key_N)
            event.accepted = true
    }

    TextMetrics {
        id: textMetrics
    }

    onModelChanged: {
        textMetrics.font = comboBox.font
        for(var i = 0; i < model.length; i++){
            textMetrics.text = model[i]
            modelWidth = Math.max(textMetrics.width, modelWidth)
        }
    }
}
