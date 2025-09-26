/**
 * @brief Test program for CUSFAM DLL functionality
 *
 * This file contains comprehensive tests for the CUSFAM DLL library,
 * including initialization, steady-state calculations, xenon dynamics,
 * shutdown margin analysis, and flexible operations.
 */

#include "CusfamDll.h"
#include <chrono>
#include <iomanip>
#include <iostream>

using namespace dnegri::cusfam::dll;
using namespace std;

/**
 * @brief Print a formatted separator line with title
 * @param title The title to display in the separator
 */
void printSeparator(const string& title) {
    cout << "\n"
         << string(50, '=') << endl;
    cout << "  " << title << endl;
    cout << string(50, '=') << endl;
}

/**
 * @brief Test CUSFAM initialization and basic functionality
 *
 * This test verifies that the CUSFAM object can be created,
 * initialized with input files, and configured with basic settings.
 * It also retrieves and displays geometry information.
 */
void testInitialization() {
    printSeparator("Initialization Test");

    try {
        // Create CUSFAM object
        Cusfam cusfam;
        cout << "✓ Cusfam object created successfully" << endl;

        // Initialize with SKN3 reactor model files
        cusfam.initialize("./run/skn3/c01/S301NOMDEP.SMG",
                          "./run/skn3/PLUS7_V127.XS",
                          "./run/skn3/PLUS7_V127.FF");

        cout << "✓ Cusfam initialized successfully" << endl;

        // Configure basic settings
        cusfam.setLogFile("cusfam_dll_test.log", 1); ///< Set log file and level
        cusfam.setIterationLimit(100, 1e-5);         ///< Set max iterations and convergence criteria
        cusfam.setNumberOfThreads(4);                ///< Set number of parallel threads
        cusfam.setTfFeedbackFactor(1.0);             ///< Set fuel temperature feedback factor
        cout << "✓ Basic settings configured" << endl;

        // Retrieve and display geometry information
        CusfamGeometry geometry = cusfam.getGeometry();
        cout << "✓ Geometry information retrieved:" << endl;
        cout << "  - nz: " << geometry.nz << endl;                  ///< Number of axial nodes
        cout << "  - nxya: " << geometry.nxya << endl;              ///< Number of radial assemblies
        cout << "  - height: " << geometry.height << " cm" << endl; ///< Core height
    } catch (const exception& e) {
        cout << "✗ Error during initialization test: " << e.what() << endl;
    }
}

/**
 * @brief Test steady-state neutronics calculation
 *
 * This test performs a complete steady-state calculation including:
 * - Setting up burnup points and control rod configurations
 * - Configuring calculation options (criticality search, xenon equilibrium, etc.)
 * - Running static calculation and pin power calculation
 * - Retrieving and displaying results
 */
void testSteadyStateCalculation() {
    printSeparator("Steady State Calculation Test");

    try {
        // Initialize CUSFAM
        Cusfam cusfam;
        cusfam.initialize("./run/skn3/c01/S301NOMDEP.SMG",
                          "./run/skn3/PLUS7_V127.XS",
                          "./run/skn3/PLUS7_V127.FF");

        // Set burnup points for depletion analysis
        vector<double> burnupPoints = {0.0, 50.0, 500.0, 1000.0, 2000.0}; ///< Burnup points in MWD/MTU
        cusfam.setBurnupPoints(burnupPoints);

        // Configure control rod groups
        cusfam.setControlRod("P");  ///< Part-length control rod
        cusfam.setControlRod("R3"); ///< Regulating rod group 3
        cusfam.setControlRod("R4"); ///< Regulating rod group 4
        cusfam.setControlRod("R5"); ///< Regulating rod group 5

        // Configure steady-state calculation options
        SteadyOption option;
        option.plevel       = 1.0;                        ///< Power level (fraction of nominal)
        option.ppm          = 500.0;                      ///< Target reactivity in pcm
        option.tin          = 290.0;                      ///< Inlet coolant temperature (°C)
        option.shpmtch      = ShapeMatchOption::SHAPE_NO; ///< No shape matching
        option.searchOption = CriticalOption::CBC;        ///< Critical boron concentration search
        option.xenon        = XEType::XE_EQ;              ///< Equilibrium xenon
        option.samarium     = SMType::SM_TR;              ///< Transient samarium
        option.feedtm       = true;                       ///< Enable moderator temperature feedback
        option.feedtf       = true;                       ///< Enable fuel temperature feedback
        option.eigvt        = 1.00000;                    ///< Target eigenvalue
        option.epsiter      = 1.E-5;                      ///< Iteration convergence criterion
        option.maxiter      = 100;                        ///< Maximum iterations

        // Set control rod positions (cm from bottom)
        option.rod_pos["P"]  = 381.0; ///< Part-length rod position
        option.rod_pos["R5"] = 381.0; ///< R5 group position
        option.rod_pos["R4"] = 381.0; ///< R4 group position
        option.rod_pos["R3"] = 381.0; ///< R3 group position

        cout << "✓ Steady state options configured" << endl;

        // Set burnup state to beginning of life
        cusfam.setBurnup("./run/skn3/c01/S301NOMDEP", burnupPoints[0], option);
        cout << "✓ Burnup state set to " << burnupPoints[0] << " MWD/MTU" << endl;

        // Perform steady-state calculation
        auto start = chrono::high_resolution_clock::now();
        cusfam.calcStatic(option); ///< Core-wide neutronics calculation
        cusfam.calcPinPower();     ///< Pin-by-pin power calculation
        auto end = chrono::high_resolution_clock::now();

        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        cout << "✓ Calculation completed in " << duration.count() << " ms" << endl;

        // Retrieve and display results
        CusfamResult result = cusfam.getResult();

        cout << "✓ Results obtained:" << endl;
        cout << fixed << setprecision(6);
        cout << "  - Eigenvalue: " << result.eigv << endl;           ///< Neutron multiplication factor
        cout << "  - PPM: " << result.ppm << " pcm" << endl;         ///< Reactivity in parts per million
        cout << "  - Fq: " << result.fq << endl;                     ///< 3D power peaking factor
        cout << "  - ASI: " << result.asi << endl;                   ///< Axial shape index
        cout << "  - Power level: " << result.plevel << "%" << endl; ///< Actual power level

    } catch (const exception& e) {
        cout << "✗ Error in steady state test: " << e.what() << endl;
    }
}

/**
 * @brief Test xenon dynamics simulation
 *
 * This test demonstrates xenon transient behavior following a power change.
 * It simulates xenon poisoning effects over a 24-hour period with 1-hour time steps.
 */
void testXenonDynamics() {
    printSeparator("Xenon Dynamics Test");

    try {
        // Initialize CUSFAM with reactor model
        Cusfam cusfam;
        cusfam.initialize("./run/skn3/c01/S301NOMDEP.SMG",
                          "./run/skn3/PLUS7_V127.XS",
                          "./run/skn3/PLUS7_V127.FF");

        // Set up burnup and control rod configuration
        vector<double> burnupPoints = {0.0, 50.0, 500.0, 1000.0, 2000.0};
        cusfam.setBurnupPoints(burnupPoints);

        cusfam.setControlRod("P");
        cusfam.setControlRod("R3");
        cusfam.setControlRod("R4");
        cusfam.setControlRod("R5");

        // Configure calculation options for transient xenon
        SteadyOption option;
        option.plevel       = 1.0;
        option.ppm          = 500.0;
        option.tin          = 290.0;
        option.shpmtch      = ShapeMatchOption::SHAPE_NO;
        option.searchOption = CriticalOption::CBC;
        option.xenon        = XEType::XE_EQ; ///< Start with equilibrium xenon
        option.samarium     = SMType::SM_TR; ///< Transient samarium
        option.feedtm       = true;
        option.feedtf       = true;
        option.eigvt        = 1.00000;
        option.epsiter      = 1.E-5;
        option.maxiter      = 100;

        option.rod_pos["P"]  = 381.0;
        option.rod_pos["R5"] = 381.0;
        option.rod_pos["R4"] = 381.0;
        option.rod_pos["R3"] = 381.0;

        cusfam.setBurnup("./run/skn3/c01/S301NOMDEP", burnupPoints[0], option);
        cout << "✓ Burnup state set to " << burnupPoints[0] << " MWD/MTU" << endl;

        // Create xenon dynamics operation
        XenonDynamicsOperation xenonOp(cusfam);
        cout << "✓ XenonDynamicsOperation created" << endl;

        // Configure time parameters
        xenonOp.setTime(3600.0 * 24, 3600.0); ///< 24 hours simulation, 1 hour time steps
        xenonOp.setXenonFactor(1.0);          ///< Xenon multiplication factor
        cout << "✓ Xenon dynamics parameters set" << endl;

        xenonOp.reset();
        cout << "✓ Xenon dynamics reset" << endl;

        // Run xenon transient simulation
        int step = 0;
        cout << "\nTime Step Results:" << endl;
        cout << "Step\tTime(h)\tKeff\t\tPPM" << endl;
        cout << string(40, '-') << endl;

        while (xenonOp.next() && step < 5) { ///< Test first 5 time steps only
            CusfamResult result = xenonOp.runStep(option);

            cout << step << "\t"
                 << fixed << setprecision(1) << result.time / 3600.0 << "\t" ///< Time in hours
                 << fixed << setprecision(6) << result.eigv << "\t"          ///< k-effective
                 << fixed << setprecision(1) << result.ppm << endl;          ///< Reactivity in pcm

            step++;
        }

        cout << "✓ Xenon dynamics simulation completed" << endl;

    } catch (const exception& e) {
        cout << "✗ Error in xenon dynamics test: " << e.what() << endl;
    }
}

/**
 * @brief Test shutdown margin analysis
 *
 * This test calculates the shutdown margin considering:
 * - Control rod worth uncertainties
 * - Stuck rod scenarios
 * - Xenon and samarium reactivity effects
 * - Power defect and temperature effects
 */
void testShutdownMargin() {
    printSeparator("Shutdown Margin Test");

    try {
        // Initialize CUSFAM
        Cusfam cusfam;
        cusfam.initialize("./run/skn3/c01/S301NOMDEP.SMG",
                          "./run/skn3/PLUS7_V127.XS",
                          "./run/skn3/PLUS7_V127.FF");

        // Set up reactor configuration
        vector<double> burnupPoints = {0.0, 50.0, 500.0, 1000.0, 2000.0};
        cusfam.setBurnupPoints(burnupPoints);

        cusfam.setControlRod("P");
        cusfam.setControlRod("R3");
        cusfam.setControlRod("R4");
        cusfam.setControlRod("R5");

        // Configure calculation options
        SteadyOption option;
        option.plevel       = 1.0;
        option.ppm          = 500.0;
        option.tin          = 290.0;
        option.shpmtch      = ShapeMatchOption::SHAPE_NO;
        option.searchOption = CriticalOption::CBC;
        option.xenon        = XEType::XE_EQ;
        option.samarium     = SMType::SM_TR;
        option.feedtm       = true;
        option.feedtf       = true;
        option.eigvt        = 1.00000;
        option.epsiter      = 1.E-5;
        option.maxiter      = 100;

        option.rod_pos["P"]  = 381.0;
        option.rod_pos["R5"] = 381.0;
        option.rod_pos["R4"] = 381.0;
        option.rod_pos["R3"] = 381.0;

        cusfam.setBurnup("./run/skn3/c01/S301NOMDEP", burnupPoints[0], option);
        cout << "✓ Burnup state set to " << burnupPoints[0] << " MWD/MTU" << endl;

        // Create shutdown margin analysis object
        ShutdownMargin sdm(cusfam);
        cout << "✓ ShutdownMargin object created" << endl;

        // Configure uncertainty parameters
        sdm.setRodUncertainty(0.06);   ///< 6% control rod worth uncertainty
        sdm.setVoidUncertainty(0.001); ///< 100 pcm void reactivity uncertainty

        // Set stuck rod scenario
        vector<string> stuckRods = {"R5"}; ///< Rods that cannot be inserted
        sdm.setStuckRods("P", stuckRods);  ///< P rod fails, R5 gets stuck
        cout << "✓ Shutdown margin parameters set" << endl;

        // Perform shutdown margin calculation
        double    timeStep = 0; ///< Steady-state analysis (no time step)
        SDMResult result   = sdm.run(timeStep, option);

        // Display shutdown margin analysis results
        cout << "✓ Shutdown margin analysis completed:" << endl;
        cout << "  - Bite Worth: " << result.biteWorth << " pcm" << endl;          ///< Total control rod worth
        cout << "  - Power Defect: " << result.powerDefect << " pcm" << endl;      ///< Power-to-zero reactivity change
        cout << "  - Stuck Rod: " << result.stuckRod << endl;                      ///< Most reactive stuck rod
        cout << "  - Stuck Rod Worth: " << result.stuckRodWorth << " pcm" << endl; ///< Worth of stuck rod
        cout << "  - Total Margin: " << result.margin << " pcm" << endl;           ///< Available shutdown margin
        cout << "  - Xenon Worth: " << result.xenonWorth << " pcm" << endl;        ///< Xenon reactivity contribution
        cout << "  - Samarium Worth: " << result.samariumWorth << " pcm" << endl;  ///< Samarium reactivity contribution

    } catch (const exception& e) {
        cout << "✗ Error in shutdown margin test: " << e.what() << endl;
    }
}

/**
 * @brief Test flexible operation transient simulation
 *
 * This test demonstrates power maneuvering capabilities including:
 * - Power level changes with automatic control rod movement
 * - Xenon transient effects during power changes
 * - Time-dependent reactor behavior
 */
void testFlexibleOperation() {
    printSeparator("Flexible Operation Test");

    try {
        // Initialize CUSFAM
        Cusfam cusfam;
        cusfam.initialize("./run/skn3/c01/S301NOMDEP.SMG",
                          "./run/skn3/PLUS7_V127.XS",
                          "./run/skn3/PLUS7_V127.FF");

        // Set up reactor configuration
        vector<double> burnupPoints = {0.0, 50.0, 500.0, 1000.0, 2000.0};
        cusfam.setBurnupPoints(burnupPoints);

        cusfam.setControlRod("P");
        cusfam.setControlRod("R3");
        cusfam.setControlRod("R4");
        cusfam.setControlRod("R5");

        // Configure calculation options
        SteadyOption option;
        option.plevel       = 1.0;
        option.ppm          = 500.0;
        option.tin          = 290.0;
        option.shpmtch      = ShapeMatchOption::SHAPE_NO;
        option.searchOption = CriticalOption::CBC;
        option.xenon        = XEType::XE_EQ;
        option.samarium     = SMType::SM_TR;
        option.feedtm       = true;
        option.feedtf       = true;
        option.eigvt        = 1.00000;
        option.epsiter      = 1.E-5;
        option.maxiter      = 100;

        option.rod_pos["P"]  = 381.0;
        option.rod_pos["R5"] = 381.0;
        option.rod_pos["R4"] = 381.0;
        option.rod_pos["R3"] = 381.0;

        cusfam.setBurnup("./run/skn3/c01/S301NOMDEP", burnupPoints[0], option);
        cout << "✓ Burnup state set to " << burnupPoints[0] << " MWD/MTU" << endl;

        // Create flexible operation object
        FlexibleOperation flexOp(cusfam);
        cout << "✓ FlexibleOperation created" << endl;

        // Configure power maneuvering schedule
        flexOp.setTimeStep(3600.0);                              ///< 1 hour time steps
        flexOp.setPowerSchedule(100.0, 50.0, 0.1, 0.05, 7200.0); ///< 100% to 50% power over 2 hours

        // Configure transient parameters
        flexOp.setXenonFactor(1.0);     ///< Xenon production factor
        flexOp.setFuelDepletion(false); ///< Disable fuel burnup for this test
        cout << "✓ Flexible operation parameters set" << endl;

        flexOp.reset();

        // Run power transient simulation
        int step = 0;
        cout << "\nPower Transient Results:" << endl;
        cout << "Step\tTime(h)\tKeff\t\tPower(%)" << endl;
        cout << string(40, '-') << endl;

        while (flexOp.next() && step < 3) { ///< Test first 3 time steps only
            CusfamResult result = flexOp.runStep(option);

            cout << step << "\t"
                 << fixed << setprecision(1) << result.time / 3600.0 << "\t" ///< Time in hours
                 << fixed << setprecision(6) << result.eigv << "\t"          ///< k-effective
                 << fixed << setprecision(1) << result.plevel << endl;       ///< Power level percentage

            step++;
        }

        cout << "✓ Flexible operation simulation completed" << endl;

    } catch (const exception& e) {
        cout << "✗ Error in flexible operation test: " << e.what() << endl;
    }
}

/**
 * @brief Main test program entry point
 *
 * Executes all CUSFAM DLL tests in sequence and reports overall results.
 * Times the execution of all tests and provides a summary.
 *
 * @return 0 on successful completion, non-zero on error
 */
int main() {
    cout << "=== CUSFAM DLL Test Program ===" << endl;
    cout << "Built on " << __DATE__ << " at " << __TIME__ << endl;

    // Record start time for performance measurement
    auto start_time = chrono::high_resolution_clock::now();

    // Execute all test functions in sequence
    testInitialization();         ///< Test basic CUSFAM initialization
    testSteadyStateCalculation(); ///< Test steady-state neutronics calculation
    testXenonDynamics();          ///< Test xenon transient simulation
    testShutdownMargin();         ///< Test shutdown margin analysis
    testFlexibleOperation();      ///< Test flexible power maneuvering
    // testCInterface();          ///< C interface test (commented out for this run)

    // Calculate and display total execution time
    auto end_time       = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::seconds>(end_time - start_time);

    printSeparator("Test Summary");
    cout << "Total test time: " << total_duration.count() << " seconds" << endl;
    cout << "All tests completed!" << endl;

    // Wait for user input before exiting
    cout << "\nPress Enter to exit...";
    cin.get();

    return 0;
}
