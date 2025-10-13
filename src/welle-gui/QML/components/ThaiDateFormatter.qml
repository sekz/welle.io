// ThaiDateFormatter.qml
// Buddhist Era (BE) date formatting for Thailand DAB+ receiver
// Converts Gregorian calendar (CE) to Buddhist calendar (BE = CE + 543)
// Usage: ThaiDateFormatter.format(dateTime, "full")

pragma Singleton
import QtQuick 2.15

QtObject {
    id: root

    // Thai month names (full)
    readonly property var thaiMonthNames: [
        "มกราคม",      // January
        "กุมภาพันธ์",   // February
        "มีนาคม",      // March
        "เมษายน",      // April
        "พฤษภาคม",     // May
        "มิถุนายน",     // June
        "กรกฎาคม",     // July
        "สิงหาคม",     // August
        "กันยายน",     // September
        "ตุลาคม",      // October
        "พฤศจิกายน",   // November
        "ธันวาคม"      // December
    ]

    // Thai month names (abbreviated)
    readonly property var thaiMonthNamesShort: [
        "ม.ค.",   // Jan
        "ก.พ.",   // Feb
        "มี.ค.",  // Mar
        "เม.ย.",  // Apr
        "พ.ค.",   // May
        "มิ.ย.",  // Jun
        "ก.ค.",   // Jul
        "ส.ค.",   // Aug
        "ก.ย.",   // Sep
        "ต.ค.",   // Oct
        "พ.ย.",   // Nov
        "ธ.ค."    // Dec
    ]

    // Thai day of week names (full)
    readonly property var thaiDayNames: [
        "วันอาทิตย์",   // Sunday
        "วันจันทร์",    // Monday
        "วันอังคาร",    // Tuesday
        "วันพุธ",       // Wednesday
        "วันพฤหัสบดี", // Thursday
        "วันศุกร์",     // Friday
        "วันเสาร์"      // Saturday
    ]

    // Thai day of week names (abbreviated)
    readonly property var thaiDayNamesShort: [
        "อา.",  // Sun
        "จ.",   // Mon
        "อ.",   // Tue
        "พ.",   // Wed
        "พฤ.",  // Thu
        "ศ.",   // Fri
        "ส."    // Sat
    ]

    // Convert Gregorian year (CE) to Buddhist Era (BE)
    function toBuddhistYear(ceYear) {
        return ceYear + 543;
    }

    // Format date and time according to Thai conventions
    // Formats:
    //   "full"     : "วันที่ 13 ตุลาคม พ.ศ. 2568 เวลา 14:30:00"
    //   "long"     : "13 ตุลาคม พ.ศ. 2568 14:30"
    //   "medium"   : "13 ต.ค. 2568 14:30"
    //   "short"    : "13/10/2568 14:30"
    //   "date"     : "13 ตุลาคม พ.ศ. 2568"
    //   "time"     : "14:30:00"
    //   "datetime" : "13/10/2568 14:30:00"
    function format(dateTime, formatType) {
        if (!dateTime || !dateTime.getTime) {
            return "";
        }

        var day = dateTime.getDate();
        var month = dateTime.getMonth(); // 0-11
        var year = toBuddhistYear(dateTime.getFullYear());
        var hours = Qt.formatTime(dateTime, "hh");
        var minutes = Qt.formatTime(dateTime, "mm");
        var seconds = Qt.formatTime(dateTime, "ss");

        switch (formatType) {
            case "full":
                return "วันที่ " + day + " " + thaiMonthNames[month] +
                       " พ.ศ. " + year + " เวลา " + hours + ":" + minutes + ":" + seconds;

            case "long":
                return day + " " + thaiMonthNames[month] + " พ.ศ. " + year +
                       " " + hours + ":" + minutes;

            case "medium":
                return day + " " + thaiMonthNamesShort[month] + " " + year +
                       " " + hours + ":" + minutes;

            case "short":
                return padZero(day) + "/" + padZero(month + 1) + "/" + year +
                       " " + hours + ":" + minutes;

            case "date":
                return day + " " + thaiMonthNames[month] + " พ.ศ. " + year;

            case "date_short":
                return padZero(day) + "/" + padZero(month + 1) + "/" + year;

            case "time":
                return hours + ":" + minutes + ":" + seconds;

            case "time_short":
                return hours + ":" + minutes;

            case "datetime":
                return padZero(day) + "/" + padZero(month + 1) + "/" + year +
                       " " + hours + ":" + minutes + ":" + seconds;

            case "day_date":
                var dayOfWeek = dateTime.getDay();
                return thaiDayNames[dayOfWeek] + "ที่ " + day + " " +
                       thaiMonthNames[month] + " พ.ศ. " + year;

            default:
                // Default: medium format
                return day + " " + thaiMonthNamesShort[month] + " " + year +
                       " " + hours + ":" + minutes;
        }
    }

    // Format relative time (e.g., "5 นาทีที่แล้ว")
    function formatRelative(dateTime) {
        if (!dateTime || !dateTime.getTime) {
            return "";
        }

        var now = new Date();
        var diffMs = now.getTime() - dateTime.getTime();
        var diffSeconds = Math.floor(diffMs / 1000);

        if (diffSeconds < 60) {
            return "เมื่อสักครู่"; // just now
        } else if (diffSeconds < 3600) {
            var minutes = Math.floor(diffSeconds / 60);
            return minutes + " นาทีที่แล้ว";
        } else if (diffSeconds < 86400) {
            var hours = Math.floor(diffSeconds / 3600);
            return hours + " ชั่วโมงที่แล้ว";
        } else if (diffSeconds < 172800) {
            return "เมื่อวาน"; // yesterday
        } else if (diffSeconds < 604800) {
            var days = Math.floor(diffSeconds / 86400);
            return days + " วันที่แล้ว";
        } else {
            // More than a week ago: show full date
            return format(dateTime, "date");
        }
    }

    // Format duration (e.g., "5:30" for 5 minutes 30 seconds)
    function formatDuration(seconds) {
        if (!seconds || seconds < 0) {
            return "0:00";
        }

        var hours = Math.floor(seconds / 3600);
        var minutes = Math.floor((seconds % 3600) / 60);
        var secs = Math.floor(seconds % 60);

        if (hours > 0) {
            return hours + ":" + padZero(minutes) + ":" + padZero(secs);
        } else {
            return minutes + ":" + padZero(secs);
        }
    }

    // Format duration with Thai units (e.g., "5 นาที 30 วินาที")
    function formatDurationThai(seconds) {
        if (!seconds || seconds < 0) {
            return "0 วินาที";
        }

        var hours = Math.floor(seconds / 3600);
        var minutes = Math.floor((seconds % 3600) / 60);
        var secs = Math.floor(seconds % 60);

        var result = [];

        if (hours > 0) {
            result.push(hours + " ชั่วโมง");
        }
        if (minutes > 0) {
            result.push(minutes + " นาที");
        }
        if (secs > 0 || result.length === 0) {
            result.push(secs + " วินาที");
        }

        return result.join(" ");
    }

    // Helper function to pad numbers with zero
    function padZero(num) {
        return num < 10 ? "0" + num : num.toString();
    }

    // Get current date/time in Thai format
    function now(formatType) {
        return format(new Date(), formatType || "medium");
    }

    // Parse ISO 8601 date string and format in Thai
    function fromISO(isoString, formatType) {
        if (!isoString) {
            return "";
        }

        var date = new Date(isoString);
        return format(date, formatType || "medium");
    }

    // Get day of week in Thai
    function getDayOfWeek(dateTime, abbreviated) {
        if (!dateTime || !dateTime.getTime) {
            return "";
        }

        var dayIndex = dateTime.getDay();
        return abbreviated ? thaiDayNamesShort[dayIndex] : thaiDayNames[dayIndex];
    }

    // Get month name in Thai
    function getMonthName(dateTime, abbreviated) {
        if (!dateTime || !dateTime.getTime) {
            return "";
        }

        var monthIndex = dateTime.getMonth();
        return abbreviated ? thaiMonthNamesShort[monthIndex] : thaiMonthNames[monthIndex];
    }
}
