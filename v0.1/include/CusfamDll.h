/**
 * @brief CUSFAM DLL Interface Header
 *
 * This header file provides the complete C++ DLL interface for the CUSFAM nuclear reactor
 * physics simulation code. It includes classes for steady-state calculations, transient
 * analysis, xenon dynamics, shutdown margin analysis, and various reactor operations.
 *
 * The DLL interface wraps the original CUSFAM code to provide a clean, platform-independent
 * API that can be used from C++, C, and other programming languages.
 */

#pragma once

#ifdef _WIN32
    #ifdef CUSFAMDLL_EXPORTS
        #define CUSFAM_API __declspec(dllexport)
    #else
        #define CUSFAM_API __declspec(dllimport)
    #endif
#elif defined(__APPLE__) || defined(__linux__)
    #ifdef CUSFAMDLL_EXPORTS
        #define CUSFAM_API __attribute__((visibility("default")))
    #else
        #define CUSFAM_API
    #endif
#else
    #define CUSFAM_API
#endif

#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace std;

/**
 * @namespace dnegri::cusfam::dll
 * @brief Main namespace for CUSFAM DLL interface
 *
 * This namespace contains all classes, structures, and enumerations
 * for the CUSFAM DLL interface, providing nuclear reactor physics
 * simulation capabilities.
 */
namespace dnegri::cusfam::dll {

/**
 * @enum ShapeMatchOption
 * @brief Options for power shape matching in calculations
 *
 * Defines how the power distribution shape should be handled
 * during neutronics calculations.
 */
enum ShapeMatchOption : int {
    SHAPE_NO    = 0, ///< No shape matching applied
    SHAPE_HOLD  = 1, ///< Hold current power shape
    SHAPE_MATCH = 2  ///< Match target power shape
};

/**
 * @enum CriticalOption
 * @brief Criticality search options for steady-state calculations
 *
 * Specifies what parameter should be adjusted to achieve criticality
 * (k-effective = 1.0) in steady-state calculations.
 */
enum CriticalOption : int {
    KEFF  = 0, ///< Search for k-effective (no parameter adjustment)
    CBC   = 1, ///< Search for critical boron concentration
    POWER = 2, ///< Search for critical power level
    ROD   = 3  ///< Search for critical control rod position
};

/**
 * @enum DepletionIsotope
 * @brief Isotope tracking options for fuel depletion calculations
 *
 * Defines which isotopes should be tracked during fuel burnup
 * and depletion calculations.
 */
enum DepletionIsotope : int {
    DEP_ALL = 0, ///< Track all depletion isotopes
    DEP_FP  = 1, ///< Track fission products only
    DEP_XE  = 2  ///< Track xenon isotopes only
};

/**
 * @enum XEType
 * @brief Xenon treatment options in neutronics calculations
 *
 * Specifies how xenon-135 should be treated in the calculation,
 * affecting reactivity and neutron absorption.
 */
enum XEType : int {
    XE_NO = 0, ///< No xenon effects
    XE_EQ = 1, ///< Equilibrium xenon concentration
    XE_TR = 2, ///< Transient xenon (time-dependent)
    XE_FX = 3  ///< Fixed xenon concentration
};

/**
 * @enum SMType
 * @brief Samarium treatment options in neutronics calculations
 *
 * Specifies how samarium-149 should be treated in the calculation,
 * affecting neutron absorption and reactivity.
 */
enum SMType : int {
    SM_NO = 0, ///< No samarium effects
    SM_TR = 1, ///< Transient samarium (time-dependent)
    SM_FX = 2  ///< Fixed samarium concentration
};

/**
 * @enum TimeUnit
 * @brief Time unit options for transient calculations
 *
 * Defines the time units used in transient and depletion calculations.
 */
enum TimeUnit : int {
    TIME_SEC  = 0, ///< Time in seconds
    TIME_HOUR = 1, ///< Time in hours
    TIME_MWD  = 2  ///< Time in megawatt-days per metric ton (burnup units)
};

/**
 * @enum ECPOption
 * @brief Emergency cooldown procedure options
 *
 * Defines the control strategy for emergency cooldown procedures.
 */
enum ECPOption : int {
    ECP_CBC = 0, ///< Control via critical boron concentration
    ECP_ROD = 1  ///< Control via control rod insertion
};

/**
 * @struct CusfamGeometry
 * @brief Reactor core geometry information
 *
 * Contains geometric parameters describing the reactor core layout,
 * including axial and radial node distributions and physical dimensions.
 */
struct CUSFAM_API CusfamGeometry {
    int            nz;     ///< Number of axial nodes
    int            kbc;    ///< Bottom plane of active fuel
    int            kec;    ///< Top plane of active fuel
    int            nxa;    ///< Number of radial assemblies in X direction
    int            nya;    ///< Number of radial assemblies in Y direction
    int            nxya;   ///< Total number of radial assemblies (nxa * nya)
    int            nyfa;   ///< Number of fuel assembly rows in Y direction
    vector<int>    nxsa;   ///< Starting X indices for each assembly row
    vector<int>    nxea;   ///< Ending X indices for each assembly row
    vector<int>    nxsfa;  ///< Starting fuel X indices for each fuel assembly row
    vector<int>    nxefa;  ///< Ending fuel X indices for each fuel assembly row
    double         height; ///< Total core height in centimeters
    vector<double> hz;     ///< Axial node heights in centimeters
};

/**
 * @struct SteadyOption
 * @brief Configuration options for steady-state calculations
 *
 * Contains all parameters needed to configure a steady-state neutronics
 * calculation, including search options, feedback parameters, and convergence criteria.
 */
struct CUSFAM_API SteadyOption {
    CriticalOption      searchOption; ///< Type of criticality search to perform
    ShapeMatchOption    shpmtch;      ///< Power shape matching option
    bool                feedtf;       ///< Enable fuel temperature feedback (true/false)
    bool                feedtm;       ///< Enable moderator temperature feedback (true/false)
    XEType              xenon;        ///< Xenon treatment option
    SMType              samarium;     ///< Samarium treatment option
    double              tin;          ///< Inlet coolant temperature in Celsius
    double              eigvt;        ///< Target eigenvalue (typically 1.0 for critical)
    int                 maxiter;      ///< Maximum number of outer iterations
    double              epsiter;      ///< Convergence criterion for outer iterations
    double              ppm;          ///< Target reactivity in parts per million (pcm)
    double              plevel;       ///< Power level as fraction of nominal (0.0-1.0)
    double              b10a;         ///< Boron-10 absorption cross-section multiplier
    double              time;         ///< Time point for transient calculations in seconds
    map<string, double> rod_pos;      ///< Control rod positions by rod ID (cm from bottom)
};

/**
 * @struct DepletionOption
 * @brief Configuration options for fuel depletion calculations
 *
 * Specifies parameters for fuel burnup and isotopic depletion calculations,
 * including time step information and isotope tracking preferences.
 */
struct CUSFAM_API DepletionOption {
    DepletionIsotope isotope;   ///< Which isotopes to track during depletion
    XEType           xenon;     ///< Xenon treatment during depletion
    SMType           samarium;  ///< Samarium treatment during depletion
    double           time;      ///< Depletion time step duration
    double           xeamp;     ///< Xenon amplification factor (typically 1.0)
    TimeUnit         time_unit; ///< Units for the time parameter
};

/**
 * @struct TransientOption
 * @brief Configuration options for transient calculations
 *
 * Contains parameters for time-dependent neutronics calculations,
 * including thermal-hydraulic conditions and control rod positions.
 */
struct CUSFAM_API TransientOption {
    double              tsec;    ///< Time step duration in seconds
    double              ppm;     ///< Reactivity insertion in parts per million
    double              plevel;  ///< Power level as fraction of nominal
    double              tin;     ///< Inlet coolant temperature in Celsius
    double              flow;    ///< Coolant flow rate as fraction of nominal
    map<string, double> rod_pos; ///< Control rod positions by rod ID (cm from bottom)
};

/**
 * @struct CusfamResult
 * @brief Results from neutronics calculations
 *
 * Contains comprehensive results from steady-state or transient calculations,
 * including eigenvalue, power distributions, and reactor physics parameters.
 */
struct CUSFAM_API CusfamResult {
    int                 nxya;    ///< Number of radial assemblies
    int                 nz;      ///< Number of axial nodes
    int                 error;   ///< Error code (0 = success, non-zero = error)
    double              eigv;    ///< Neutron multiplication factor (k-effective)
    double              ppm;     ///< Reactivity in parts per million (pcm)
    double              fq;      ///< 3D power peaking factor (hottest pin/average)
    double              fxy;     ///< Radial power peaking factor
    double              fr;      ///< Assembly power peaking factor
    double              fz;      ///< Axial power peaking factor
    double              asi;     ///< Axial shape index
    double              tf;      ///< Average fuel temperature in Celsius
    double              tm;      ///< Average moderator temperature in Celsius
    double              plevel;  ///< Actual power level as fraction of nominal
    vector<double>      pow2d;   ///< Assembly-wise power distribution (nxya elements)
    vector<double>      pow1d;   ///< Axial power distribution (nz elements)
    double              time;    ///< Calculation time point in seconds
    double              burnup;  ///< Fuel burnup in MWD/MTU
    map<string, double> rod_pos; ///< Final control rod positions by rod ID (cm)

    /**
     * @brief Initialize result arrays
     * @param nxya Number of radial assemblies
     * @param nz Number of axial nodes
     *
     * Allocates memory for power distribution arrays based on core geometry.
     */
    void init(int nxya, int nz);

    /**
     * @brief Generate additional result parameters
     *
     * Calculates derived parameters from basic results (currently placeholder).
     */
    void generate();
};

/**
 * @struct ScenarioItem
 * @brief Single item in a power maneuvering scenario
 *
 * Defines one step in a multi-step power transient scenario, including
 * duration, target power, and reactivity control parameters.
 */
struct CUSFAM_API ScenarioItem {
    static constexpr double TARGET_INITIAL_ESI = -1.0; ///< Default initial ESI target

    double               duration;     ///< Duration of this scenario step in seconds
    double               powerRatio;   ///< Target power as fraction of nominal (0.0-1.0)
    pair<double, double> asiAllowance; ///< Allowable ASI range (min, max)
    double               targetASI;    ///< Target axial shape index
    bool                 controlASI;   ///< Whether to actively control ASI (true/false)
};

/**
 * @struct SDMResult
 * @brief Results from shutdown margin analysis
 *
 * Contains detailed breakdown of shutdown margin components, including
 * control rod worth, uncertainties, and safety margins.
 */
struct CUSFAM_API SDMResult {
    double biteWorth;     ///< Total available control rod worth in pcm
    double powerDefect;   ///< Power defect (reactivity change from full power to zero) in pcm
    string stuckRod;      ///< Identifier of the most reactive stuck rod
    double stuckRodWorth; ///< Reactivity worth of the stuck rod in pcm
    double margin;        ///< Net shutdown margin in pcm
    double xenonWorth;    ///< Xenon reactivity contribution in pcm
    double samariumWorth; ///< Samarium reactivity contribution in pcm
    double boronWorth;    ///< Boron reactivity worth in pcm
    double tmWorth;       ///< Moderator temperature reactivity worth in pcm
};

// Forward declarations
class XenonDynamicsOperation;
class FlexibleOperation;
class CoastdownOperation;
class ECPOperation;
class StartupOperation;
class GeneralOperation;
class ShutdownMargin;

/**
 * @class Cusfam
 * @brief Main CUSFAM reactor physics calculation engine
 *
 * The primary class for nuclear reactor physics calculations. Provides methods
 * for initialization, steady-state calculations, transient analysis, and
 * fuel depletion simulations. This class serves as the foundation for all
 * other operation classes.
 */
class CUSFAM_API Cusfam {
private:
    void* pImpl; ///< Pointer to implementation (PIMPL idiom)

    // Friend classes for internal access
    friend class XenonDynamicsOperation;
    friend class FlexibleOperation;
    friend class CoastdownOperation;
    friend class ECPOperation;
    friend class StartupOperation;
    friend class GeneralOperation;
    friend class ShutdownMargin;

public:
    /**
     * @brief Default constructor
     *
     * Creates a new CUSFAM calculation engine instance.
     * Must call initialize() before performing calculations.
     */
    Cusfam();

    /**
     * @brief Destructor
     *
     * Cleans up internal resources and memory allocations.
     */
    ~Cusfam();

    /**
     * @brief Initialize CUSFAM with input files
     * @param gmt_file Path to geometry file (.SMG format)
     * @param tset_file Path to cross-section library file (.XS format)
     * @param ff_file Path to form function file (.FF format)
     *
     * Loads reactor geometry, cross-section data, and form functions
     * required for neutronics calculations. Must be called before
     * any calculation methods.
     */
    void initialize(const string& gmt_file, const string& tset_file, const string& ff_file);

    /**
     * @brief Set output log file and verbosity level
     * @param log_file Path to output log file
     * @param log_level Verbosity level (0=none, 1=basic, 2=detailed, 3=debug)
     *
     * Configures calculation logging for debugging and monitoring purposes.
     */
    void setLogFile(const string& log_file, int log_level);

    /**
     * @brief Set iteration limits and convergence criteria
     * @param maxls Maximum number of linear system iterations
     * @param epsls Convergence tolerance for linear system solver
     *
     * Controls the accuracy and computational cost of neutronics calculations.
     * Smaller epsls values give more accurate results but take longer.
     */
    void setIterationLimit(int maxls, double epsls);

    /**
     * @brief Set number of parallel computation threads
     * @param nthreads Number of OpenMP threads to use (0 = automatic)
     *
     * Configures parallel processing for improved calculation performance.
     * Optimal value depends on available CPU cores and memory bandwidth.
     */
    void setNumberOfThreads(int nthreads);

    /**
     * @brief Set fuel temperature feedback multiplier
     * @param factor Multiplication factor for fuel temperature feedback (typically 1.0)
     *
     * Adjusts the strength of Doppler reactivity feedback from fuel temperature changes.
     * Values less than 1.0 reduce feedback strength, greater than 1.0 increase it.
     */
    void setTfFeedbackFactor(double factor);

    /**
     * @brief Set burnup points for depletion calculations
     * @param burnups Vector of burnup points in MWD/MTU
     *
     * Defines the fuel exposure levels for which cross-section data is available.
     * Enables fuel depletion and burnup-dependent calculations.
     */
    void setBurnupPoints(const vector<double>& burnups);

    /**
     * @brief Set fuel temperature table for feedback calculations
     * @param burnup Burnup points in MWD/MTU
     * @param power Power levels as fraction of nominal
     * @param tftable 2D table of fuel temperatures [burnup][power] in Celsius
     *
     * Provides fuel temperature data for accurate Doppler feedback calculations
     * as a function of both burnup and power level.
     */
    void setTfTable(const vector<double>& burnup, const vector<double>& power,
                    const vector<vector<double>>& tftable);

    /**
     * @brief Get reactor geometry information
     * @return CusfamGeometry structure containing core layout parameters
     *
     * Returns geometric parameters including node counts, dimensions,
     * and boundary conditions. Useful for setting up result arrays
     * and understanding core structure.
     */
    CusfamGeometry getGeometry();

    /**
     * @brief Get results from the last calculation
     * @return CusfamResult structure containing calculation results
     *
     * Returns comprehensive results including eigenvalue, power distributions,
     * peaking factors, and other reactor physics parameters from the most
     * recent calculation.
     */
    CusfamResult getResult();

    /**
     * @brief Set burnup state and update cross-sections
     * @param dir_burnup Directory path containing burnup-dependent data files
     * @param burnup Target burnup in MWD/MTU
     * @param option Calculation options (may be modified based on burnup state)
     * @return Updated SteadyOption with burnup-appropriate parameters
     *
     * Loads cross-section data corresponding to the specified fuel burnup
     * and updates calculation parameters as needed for the new fuel state.
     */
    SteadyOption setBurnup(const string& dir_burnup, double burnup, SteadyOption& option);

    /**
     * @brief Update burnup state based on power history
     *
     * Advances fuel burnup based on the current power level and operating time.
     * Updates isotopic concentrations and cross-sections accordingly.
     */
    void updateBurnup();

    /**
     * @brief Save current reactor state to snapshot
     * @param id Unique identifier for the snapshot
     * @return Status code (0 = success, non-zero = error)
     *
     * Saves the complete reactor state (flux, power, isotopics, etc.)
     * for later restoration. Useful for branching calculations or
     * returning to a previous state.
     */
    int saveSnapshot(int id);

    /**
     * @brief Load reactor state from saved snapshot
     * @param id Identifier of the snapshot to restore
     *
     * Restores the complete reactor state from a previously saved snapshot.
     * All current calculation data is replaced with the snapshot data.
     */
    void loadSnapshot(int id);

    /**
     * @brief Run test calculation with given options
     * @param option Configuration for the test calculation
     *
     * Performs a basic neutronics calculation for testing purposes.
     * Useful for verifying setup and troubleshooting before
     * production calculations.
     */
    void runForTest(const SteadyOption& option);

    /**
     * @brief Perform steady-state neutronics calculation
     * @param option Configuration options for the calculation
     *
     * Solves the steady-state neutron diffusion equation with the specified
     * parameters. This is the primary method for critical calculations,
     * boron searches, and power distribution analysis.
     */
    void calcStatic(const SteadyOption& option);

    /**
     * @brief Search for control rod positions to achieve target ASI
     * @param option Calculation configuration options
     * @param target_asi Target axial shape index value
     *
     * Automatically adjusts control rod positions to achieve the specified
     * axial shape index while maintaining criticality. Used for axial
     * power distribution control.
     */
    void searchASI(const SteadyOption& option, double target_asi);

    /**
     * @brief Reset ASI search parameters to defaults
     *
     * Clears any ASI search constraints and returns to normal calculation mode.
     */
    void resetASI();

    /**
     * @brief Set target axial power shape
     * @param hzshp Axial height points in centimeters
     * @param powshp Relative power values at each height point
     *
     * Defines a target axial power distribution for shape matching calculations.
     * The power values are normalized internally.
     */
    void setPowerShape(const vector<double>& hzshp, const vector<double>& powshp);

    /**
     * @brief Calculate detailed pin-by-pin power distribution
     *
     * Performs fine-mesh power reconstruction to obtain pin-level power
     * distribution from the coarse-mesh solution. Required for detailed
     * thermal analysis and hot spot identification.
     */
    void calcPinPower();

    /**
     * @brief Set control rod position
     * @param rodid Control rod identifier string
     * @param position Rod position in centimeters from bottom of core
     * @param overlap Whether this rod overlaps with other rod groups
     *
     * Sets the position of a specific control rod or rod group.
     * Position of 0 = fully inserted, maximum = fully withdrawn.
     */
    void setRodPosition(const string& rodid, double position, bool overlap);

    /**
     * @brief Set control rod reactivity worth by rod ID
     * @param rodid Control rod identifier string
     * @param strength Rod worth in absolute units
     *
     * Directly specifies the reactivity worth of a control rod,
     * overriding calculated values. Used for rod worth adjustments
     * or when measured data is available.
     */
    void setRodStrengthById(const string& rodid, double strength);

    /**
     * @brief Set control rod reactivity worth by location index
     * @param la Assembly location index
     * @param strength Rod worth in absolute units
     *
     * Sets rod worth for a specific assembly location using
     * numerical indexing rather than rod ID strings.
     */
    void setRodStrengthByLoc(int la, double strength);

    /**
     * @brief Set reactivity worth for all control rods
     * @param strength Vector of rod worth values in absolute units
     *
     * Sets the reactivity worth for all control rod locations
     * simultaneously. Vector size must match number of rod locations.
     */
    void setRodStrengthAll(const vector<double>& strength);

    /**
     * @brief Perform fuel depletion calculation
     * @param xe_option Xenon treatment option (0=none, 1=equilibrium, 2=transient)
     * @param sm_option Samarium treatment option (0=none, 1=transient, 2=fixed)
     * @param del_burnup Burnup increment in MWD/MTU
     *
     * Advances fuel isotopic composition by the specified burnup increment,
     * accounting for fission, neutron absorption, and radioactive decay.
     */
    void deplete(int xe_option, int sm_option, double del_burnup);

    /**
     * @brief Perform time-based fuel depletion
     * @param xe_option Xenon treatment option
     * @param sm_option Samarium treatment option
     * @param tsec Depletion time in seconds
     * @param xeamp Xenon amplification factor
     *
     * Advances fuel composition based on operating time rather than
     * burnup increment. Power level determines the depletion rate.
     */
    void depleteByTime(int xe_option, int sm_option, double tsec, double xeamp);

    /**
     * @brief Update xenon and samarium concentrations only
     * @param xe_option Xenon treatment option
     * @param sm_option Samarium treatment option
     * @param tsec Time step in seconds
     * @param xeamp Xenon amplification factor
     *
     * Updates only xenon-135 and samarium-149 concentrations without
     * full fuel depletion. Used for poison transient calculations.
     */
    void depleteXeSm(int xe_option, int sm_option, double tsec, double xeamp);

    /**
     * @brief Set axial shape index control bands
     * @param asiBand Map of power levels to ASI control bands (min, max)
     *
     * Defines acceptable ASI ranges as a function of power level
     * for automatic shape control during power operations.
     */
    void setASIBand(const map<double, pair<double, double>>& asiBand);

    /**
     * @brief Set ASI operational allowances
     * @param asiAllowance Map of power levels to ASI allowances (min, max)
     *
     * Sets operational limits for axial shape index that trigger
     * corrective actions during power maneuvering.
     */
    void setASIAllowance(const map<double, pair<double, double>>& asiAllowance);

    /**
     * @brief Configure control rod properties
     * @param rodId Control rod identifier string
     * @param overlappedWith ID of overlapping rod group (empty if none)
     * @param range Operating range limits (bottom, top) in centimeters
     *
     * Defines control rod characteristics including overlap relationships
     * and physical movement limits for realistic rod operation modeling.
     */
    void setControlRod(const string& rodId, const string& overlappedWith = "",
                       const pair<double, double>& range = {0.0, 0.0});

    /**
     * @brief Set Power Dependent Insertion Limits (PDIL)
     * @param rodId Control rod identifier string
     * @param pdil Vector of (power_level, max_insertion) pairs
     *
     * Defines power-dependent insertion limits for control rods
     * to maintain adequate shutdown margin and power distribution
     * control at all power levels.
     */
    void setPDIL(const string& rodId, const vector<pair<double, double>>& pdil);

    /**
     * @brief Get PDIL limit for specified rod and power level
     * @param rodId Control rod identifier string
     * @param relPower Relative power level (0.0 to 1.0)
     * @return Maximum allowed insertion depth in centimeters
     *
     * Returns the power-dependent insertion limit for the specified
     * control rod at the given power level, interpolating between
     * defined PDIL points as necessary.
     */
    double getPDIL(const string& rodId, double relPower);
};

/**
 * @class XenonDynamicsOperation
 * @brief Xenon transient simulation class
 *
 * Simulates xenon-135 concentration changes following power level changes.
 * Tracks xenon buildup and decay effects on reactor reactivity over time.
 */
class CUSFAM_API XenonDynamicsOperation {
private:
    void* pImpl; ///< Pointer to implementation

public:
    /**
     * @brief Constructor
     * @param cusfam Reference to initialized CUSFAM engine
     *
     * Creates xenon dynamics simulation using the specified CUSFAM engine.
     * The engine must be initialized before creating this object.
     */
    XenonDynamicsOperation(Cusfam& cusfam);

    /**
     * @brief Destructor
     *
     * Cleans up xenon dynamics simulation resources.
     */
    ~XenonDynamicsOperation();

    /**
     * @brief Reset simulation to initial conditions
     *
     * Resets time to zero and xenon concentrations to initial values.
     * Must be called before starting a new simulation sequence.
     */
    void reset();

    /**
     * @brief Set simulation time parameters
     * @param endTime Total simulation time in seconds
     * @param timeStep Time step size in seconds
     *
     * Configures the time range and resolution for the xenon transient
     * simulation. Smaller time steps give more accurate results.
     */
    void setTime(double endTime, double timeStep);

    /**
     * @brief Set xenon production amplification factor
     * @param factor Xenon yield multiplier (typically 1.0)
     *
     * Adjusts xenon production rate for sensitivity studies or
     * to account for measurement uncertainties in xenon yield.
     */
    void setXenonFactor(double factor);

    /**
     * @brief Check if more time steps are available
     * @return true if simulation can continue, false if complete
     *
     * Returns whether the simulation has reached the specified end time.
     * Used to control the main simulation loop.
     */
    bool next() const;

    /**
     * @brief Execute one time step of xenon dynamics
     * @param stdopt Steady-state calculation options for this time step
     * @return CusfamResult containing results for this time step
     *
     * Advances xenon concentrations by one time step and performs
     * a steady-state calculation with the updated xenon levels.
     */
    CusfamResult runStep(const SteadyOption& stdopt);
};

/**
 * @class FlexibleOperation
 * @brief Flexible power maneuvering operation class
 *
 * Simulates complex power transients with automatic control rod movement
 * and reactivity management. Supports power schedules, ramp rates, and
 * axial shape control during power changes.
 */
class CUSFAM_API FlexibleOperation {
private:
    void* pImpl; ///< Pointer to implementation

public:
    /**
     * @brief Constructor
     * @param cusfam Reference to initialized CUSFAM engine
     *
     * Creates flexible operation simulation using the specified CUSFAM engine.
     */
    FlexibleOperation(Cusfam& cusfam);

    /**
     * @brief Virtual destructor
     *
     * Enables proper cleanup in derived classes.
     */
    virtual ~FlexibleOperation();

    /**
     * @brief Reset operation to initial conditions
     *
     * Resets time and power level to starting values.
     * Must be called before beginning a new power transient.
     */
    void reset();

    /**
     * @brief Set time step size for the simulation
     * @param timeStep Time step duration in seconds
     *
     * Sets the temporal resolution for the power transient simulation.
     * Smaller steps provide better accuracy but longer computation time.
     */
    void setTimeStep(double timeStep);

    /**
     * @brief Set automatic power schedule parameters
     * @param initialPower Starting power level (% of nominal)
     * @param targetPower Ending power level (% of nominal)
     * @param powerDownRatio Maximum power reduction rate (%/minute)
     * @param powerUpRatio Maximum power increase rate (%/minute)
     * @param duration Total maneuver duration in seconds
     * @param beforeTime Equilibration time before maneuver (seconds)
     * @param afterTime Equilibration time after maneuver (seconds)
     * @param asiAllowance Allowable ASI deviation during maneuver
     *
     * Configures an automatic power change from initial to target power
     * with specified ramp rates and equilibration periods.
     */
    void setPowerSchedule(double initialPower, double targetPower, double powerDownRatio,
                          double powerUpRatio, double duration, double beforeTime = 2.0 * 3600,
                          double afterTime = 2.0 * 3600, double asiAllowance = 0.01);

    /**
     * @brief Set detailed power scenario with multiple steps
     * @param scn Vector of ScenarioItem objects defining the power profile
     *
     * Defines a complex multi-step power transient with different
     * power levels, durations, and control parameters for each step.
     */
    void setPowerScenario(const vector<ScenarioItem>& scn);

    /**
     * @brief Set xenon production amplification factor
     * @param factor Xenon yield multiplier (typically 1.0)
     *
     * Adjusts xenon dynamics during the power transient for
     * sensitivity analysis or uncertainty assessment.
     */
    void setXenonFactor(double factor);

    /**
     * @brief Set control rod insertion sequence
     * @param rodIds Vector of rod group identifiers in insertion order
     * @param rodLimits Vector of insertion limits for each rod group (cm)
     *
     * Defines the order and limits for automatic control rod insertion
     * during power reductions or reactivity control.
     */
    void setRodInSequence(const vector<string>& rodIds, const vector<double>& rodLimits);

    /**
     * @brief Set control rod withdrawal sequence
     * @param rodIds Vector of rod group identifiers in withdrawal order
     * @param rodLimits Vector of withdrawal limits for each rod group (cm)
     *
     * Defines the order and limits for automatic control rod withdrawal
     * during power increases or reactivity management.
     */
    void setRodOutSequence(const vector<string>& rodIds, const vector<double>& rodLimits);

    /**
     * @brief Enable or disable fuel depletion during transient
     * @param fuelDepletion true to include burnup effects, false to ignore
     *
     * Controls whether fuel isotopic changes are calculated during
     * the power transient. Typically disabled for short transients.
     */
    void setFuelDepletion(bool fuelDepletion);

    /**
     * @brief Check if more time steps are available
     * @return true if simulation can continue, false if complete
     *
     * Returns whether the power transient simulation has completed
     * all scheduled steps and equilibration periods.
     */
    bool next() const;

    /**
     * @brief Execute one time step of flexible operation
     * @param stdopt Steady-state calculation options for this time step
     * @return CusfamResult containing results for this time step
     *
     * Advances the power transient by one time step, adjusting power level
     * and control rod positions according to the defined schedule.
     */
    CusfamResult runStep(const SteadyOption& stdopt);
};

/**
 * @class CoastdownOperation
 * @brief Reactor coastdown simulation class
 *
 * Simulates reactor power coastdown scenarios where power is gradually
 * reduced to a target level with automatic control rod management.
 */
class CUSFAM_API CoastdownOperation {
private:
    void* pImpl; ///< Pointer to implementation

public:
    /**
     * @brief Constructor
     * @param cusfam Reference to initialized CUSFAM engine
     */
    CoastdownOperation(Cusfam& cusfam);

    /**
     * @brief Destructor
     */
    ~CoastdownOperation();

    /**
     * @brief Reset coastdown simulation to initial conditions
     */
    void reset();

    /**
     * @brief Set simulation time parameters
     * @param endTime Total coastdown time in seconds
     * @param timeStep Time step size in seconds
     */
    void setTime(double endTime, double timeStep);

    /**
     * @brief Set target power level for coastdown
     * @param targetPower Final power level as fraction of nominal (0.0-1.0)
     */
    void setTargetPower(double targetPower);

    /**
     * @brief Set xenon production amplification factor
     * @param factor Xenon yield multiplier (typically 1.0)
     */
    void setXenonFactor(double factor);

    /**
     * @brief Set control rod insertion sequence
     * @param rodIds Vector of rod group identifiers in insertion order
     * @param rodLimits Vector of insertion limits for each rod group (cm)
     */
    void setRodInSequence(const vector<string>& rodIds, const vector<double>& rodLimits);

    /**
     * @brief Set control rod withdrawal sequence
     * @param rodIds Vector of rod group identifiers in withdrawal order
     * @param rodLimits Vector of withdrawal limits for each rod group (cm)
     */
    void setRodOutSequence(const vector<string>& rodIds, const vector<double>& rodLimits);

    /**
     * @brief Check if more time steps are available
     * @return true if simulation can continue, false if complete
     */
    bool next() const;

    /**
     * @brief Execute one time step of coastdown operation
     * @param stdopt Steady-state calculation options for this time step
     * @return CusfamResult containing results for this time step
     */
    CusfamResult runStep(const SteadyOption& stdopt);
};

/**
 * @class ECPOperation
 * @brief Emergency Cooldown Procedure simulation class
 *
 * Simulates emergency cooldown procedures with rapid power reduction
 * and control rod insertion following reactor trip or emergency conditions.
 */
class CUSFAM_API ECPOperation {
private:
    void* pImpl; ///< Pointer to implementation

public:
    /**
     * @brief Constructor
     * @param cusfam Reference to initialized CUSFAM engine
     */
    ECPOperation(Cusfam& cusfam);

    /**
     * @brief Destructor
     */
    ~ECPOperation();

    /**
     * @brief Reset ECP simulation to initial conditions
     */
    void reset();

    /**
     * @brief Set emergency cooldown control option
     * @param option ECP control strategy (CBC or ROD)
     */
    void setOption(ECPOption option);

    /**
     * @brief Set time parameters for emergency cooldown
     * @param endTime Total simulation time in seconds
     * @param shutdownTime Time to complete shutdown in seconds
     * @param timeStep Time step size in seconds
     */
    void setTime(double endTime, double shutdownTime, double timeStep);

    /**
     * @brief Set target critical boron concentration
     * @param targetCBC Target boron concentration in ppm
     */
    void setTargetCBC(double targetCBC);

    /**
     * @brief Set xenon production amplification factor
     * @param factor Xenon yield multiplier (typically 1.0)
     */
    void setXenonFactor(double factor);

    /**
     * @brief Set control rod insertion sequence for emergency
     * @param rodIds Vector of rod group identifiers in insertion order
     * @param rodLimits Vector of insertion limits for each rod group (cm)
     */
    void setRodInSequence(const vector<string>& rodIds, const vector<double>& rodLimits);

    /**
     * @brief Set control rod withdrawal sequence (if needed)
     * @param rodIds Vector of rod group identifiers in withdrawal order
     * @param rodLimits Vector of withdrawal limits for each rod group (cm)
     */
    void setRodOutSequence(const vector<string>& rodIds, const vector<double>& rodLimits);

    /**
     * @brief Check if more time steps are available
     * @return true if simulation can continue, false if complete
     */
    bool next() const;

    /**
     * @brief Execute one time step of ECP operation
     * @param stdopt Steady-state calculation options for this time step
     * @return CusfamResult containing results for this time step
     */
    CusfamResult runStep(const SteadyOption& stdopt);
};

/**
 * @class StartupOperation
 * @brief Reactor startup simulation class (inherits from FlexibleOperation)
 *
 * Specialized class for reactor startup procedures from shutdown conditions
 * to full power operation, including criticality approach and power escalation.
 */
class CUSFAM_API StartupOperation : public FlexibleOperation {
private:
    void* pImpl; ///< Pointer to implementation

public:
    /**
     * @brief Constructor
     * @param cusfam Reference to initialized CUSFAM engine
     */
    StartupOperation(Cusfam& cusfam);

    /**
     * @brief Destructor
     */
    ~StartupOperation();

    /**
     * @brief Reset startup simulation to initial conditions
     */
    void reset();

    /**
     * @brief Set shutdown duration before startup
     * @param shutdownTime Duration reactor was shutdown in seconds
     *
     * Sets the time the reactor was shut down, affecting xenon and
     * samarium decay calculations for startup conditions.
     */
    void setShutdownTime(double shutdownTime);

    /**
     * @brief Set initial control rod positions at startup
     * @param initialRodPos Map of rod IDs to initial positions (cm from bottom)
     *
     * Defines the control rod configuration at the beginning of
     * the startup procedure, typically with most rods inserted.
     */
    void setInitialRodPosition(const map<string, double>& initialRodPos);

    /**
     * @brief Execute one time step of startup operation
     * @param stdopt Steady-state calculation options for this time step
     * @return CusfamResult containing results for this time step
     *
     * Advances the startup procedure by one time step, managing
     * approach to criticality and power escalation.
     */
    CusfamResult runStep(const SteadyOption& stdopt);
};

/**
 * @class GeneralOperation
 * @brief General-purpose reactor operation simulation class
 *
 * Provides flexible simulation capability for custom reactor operations
 * combining steady-state calculations with fuel depletion over time.
 */
class CUSFAM_API GeneralOperation {
private:
    void* pImpl; ///< Pointer to implementation

public:
    /**
     * @brief Constructor
     * @param cusfam Reference to initialized CUSFAM engine
     */
    GeneralOperation(Cusfam& cusfam);

    /**
     * @brief Destructor
     */
    ~GeneralOperation();

    /**
     * @brief Reset operation to initial conditions
     */
    void reset();

    /**
     * @brief Set xenon production amplification factor
     * @param factor Xenon yield multiplier (typically 1.0)
     */
    void setXenonFactor(double factor);

    /**
     * @brief Set control rod insertion sequence
     * @param rodIds Vector of rod group identifiers in insertion order
     * @param rodLimits Vector of insertion limits for each rod group (cm)
     */
    void setRodInSequence(const vector<string>& rodIds, const vector<double>& rodLimits);

    /**
     * @brief Set control rod withdrawal sequence
     * @param rodIds Vector of rod group identifiers in withdrawal order
     * @param rodLimits Vector of withdrawal limits for each rod group (cm)
     */
    void setRodOutSequence(const vector<string>& rodIds, const vector<double>& rodLimits);

    /**
     * @brief Execute one calculation step with depletion
     * @param stdopt Steady-state calculation options
     * @param depopt Depletion calculation options
     * @return CusfamResult containing results for this step
     *
     * Performs a steady-state calculation followed by fuel depletion
     * for the specified time step and depletion parameters.
     */
    CusfamResult runStep(const SteadyOption& stdopt, const DepletionOption& depopt);
};

/**
 * @class ShutdownMargin
 * @brief Shutdown margin analysis class
 *
 * Calculates reactor shutdown margin considering control rod worth,
 * stuck rod scenarios, reactivity uncertainties, and operating conditions.
 * Ensures adequate subcriticality in accident scenarios.
 */
class CUSFAM_API ShutdownMargin {
private:
    void* pImpl; ///< Pointer to implementation

public:
    /**
     * @brief Constructor
     * @param cusfam Reference to initialized CUSFAM engine
     */
    ShutdownMargin(Cusfam& cusfam);

    /**
     * @brief Destructor
     */
    ~ShutdownMargin();

    /**
     * @brief Reset analysis to initial conditions
     */
    void reset();

    /**
     * @brief Set control rod worth uncertainty
     * @param uncertainty Rod worth uncertainty as fraction (e.g., 0.06 for 6%)
     *
     * Sets the uncertainty in control rod reactivity worth calculations
     * used to establish conservative shutdown margin estimates.
     */
    void setRodUncertainty(double uncertainty);

    /**
     * @brief Set void reactivity uncertainty
     * @param uncertainty Void coefficient uncertainty in absolute reactivity units
     *
     * Sets the uncertainty in void reactivity coefficient used for
     * conservative margin calculations in loss-of-coolant scenarios.
     */
    void setVoidUncertainty(double uncertainty);

    /**
     * @brief Set stuck rod scenario parameters
     * @param failedRod Identifier of the rod assumed to fail (most reactive)
     * @param stuckRods Vector of rod IDs that cannot be inserted
     *
     * Defines the stuck rod scenario for shutdown margin analysis,
     * assuming the most reactive rod fails and others may stick.
     */
    void setStuckRods(const std::string& failedRod, const std::vector<std::string>& stuckRods);

    /**
     * @brief Perform shutdown margin analysis
     * @param dt Time step for transient effects (0 for steady-state)
     * @param option Calculation options (may be modified during analysis)
     * @return SDMResult containing detailed shutdown margin breakdown
     *
     * Calculates comprehensive shutdown margin including all reactivity
     * effects, uncertainties, and stuck rod penalties. Returns detailed
     * breakdown of margin components for safety evaluation.
     */
    SDMResult run(double dt, SteadyOption& option);
};

} // namespace dnegri::cusfam::dll
