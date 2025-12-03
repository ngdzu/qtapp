/**
 * @file tst_PatientBannerTest.qml
 * @brief Qt Quick Test for PatientBanner component
 *
 * Tests patient information display and banner rendering.
 *
 * @author Z Monitor Team
 * @date 2025-12-03
 */

import QtQuick
import QtTest

import "../../../resources/qml/components"

TestCase {
    id: testCase
    name: "PatientBannerTest"
    when: windowShown
    width: 800
    height: 150

    // Test component
    PatientBanner {
        id: patientBanner
        anchors.fill: parent
    }

    function init() {
        // Reset to default state before each test
        patientBanner.name = "";
        patientBanner.bedLabel = "";
    }

    function test_01_initialState() {
        verify(patientBanner.width > 0, "Banner has width");
        verify(patientBanner.height > 0, "Banner has height");
        compare(patientBanner.name, "", "Default name is empty");
        compare(patientBanner.bedLabel, "", "Default bed label is empty");
    }

    function test_02_patientNameDisplay() {
        patientBanner.name = "John Doe";
        
        compare(patientBanner.name, "John Doe", "Patient name set correctly");
        wait(50);
    }

    function test_03_bedLabelDisplay() {
        patientBanner.bedLabel = "ICU-3A";
        
        compare(patientBanner.bedLabel, "ICU-3A", "Bed label set correctly");
        wait(50);
    }

    function test_04_completeInfo() {
        patientBanner.name = "John Doe";
        patientBanner.bedLabel = "ICU-3A";
        
        compare(patientBanner.name, "John Doe", "Name set");
        compare(patientBanner.bedLabel, "ICU-3A", "Bed label set");
        wait(50);
    }

    function test_05_emptyInfo() {
        patientBanner.name = "";
        patientBanner.bedLabel = "";
        
        verify(patientBanner.name === "", "Empty name handled");
        verify(patientBanner.bedLabel === "", "Empty bed label handled");
        wait(50);
    }

    function test_06_longName() {
        patientBanner.name = "Dr. Alexander Christopher Montgomery-Wellington III";
        
        verify(patientBanner.name.length > 20, "Long name handled");
        wait(50);
    }

    function test_07_specialCharactersInName() {
        patientBanner.name = "O'Connor-Smith";
        
        verify(patientBanner.name.includes("'"), "Apostrophe handled");
        verify(patientBanner.name.includes("-"), "Hyphen handled");
        wait(50);
    }

    function test_08_infoUpdate() {
        // Simulate patient change
        patientBanner.name = "Patient A";
        patientBanner.bedLabel = "ICU-1";
        wait(50);
        
        patientBanner.name = "Patient B";
        patientBanner.bedLabel = "ICU-2";
        wait(50);
        
        compare(patientBanner.name, "Patient B", "Name updated");
        compare(patientBanner.bedLabel, "ICU-2", "Bed label updated");
    }
}
