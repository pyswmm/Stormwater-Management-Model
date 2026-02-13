# SWMM Narrative Specification Document

## Executive Summary

The Storm Water Management Model (SWMM) is a dynamic, physically-based, hydrology-hydraulic-water quality simulation model developed for planning, analysis, and design related to stormwater runoff, combined and sanitary sewers, and other drainage systems in urban and non-urban areas. This document provides a comprehensive narrative specification of the SWMM computational engine, its architecture, capabilities, and key components.

## 1. Introduction

### 1.1 Purpose and Scope

SWMM is maintained by the Open Water Analytics (OWA) group as an open-source project. The model is designed for single-event or continuous (long-term) simulation of runoff quantity and quality from primarily urban areas. The computational engine is written in C and released under a combination of MIT License and Public Domain licensing.

### 1.2 Version Information

- **Current Version**: 5.2.4
- **Original Development**: U.S. Environmental Protection Agency (USEPA)
- **Current Maintainers**: Open Water Analytics community
- **Primary Language**: C Programming Language
- **Build System**: CMake (version 3.23.0 or higher)

### 1.3 Key Contributors

The SWMM project represents decades of development and refinement by numerous contributors:
- **Lewis Rossman**: Original developer and legacy maintainer (Public Domain)
- **Bryant McDonnell**: Founder and current maintainer, toolkit API development
- **Michael Tryby**: Development operations, output library, testing framework
- **Jia Xin Wu**: Maintainer, LID API, bug fixes, Python wrapper
- **Katherine Ratliff**: Stats API development and testing
- Additional contributions from the broader OWA community

## 2. System Overview

### 2.1 Model Capabilities

SWMM provides comprehensive simulation capabilities for:

#### Hydrology
- Rainfall-runoff simulation from multiple subcatchments
- Snowfall accumulation and snowmelt modeling
- Surface storage and depression storage
- Multiple infiltration methods (Horton, Green-Ampt, Curve Number)
- Groundwater flow interactions
- Low Impact Development (LID) practices simulation
- Rainfall-Dependent Inflow/Infiltration (RDII)

#### Hydraulics
- Network flow routing using kinematic wave or dynamic wave methods
- Conduit, pump, orifice, weir, and outlet hydraulics
- Pressure flow in closed conduits
- Pond and storage facility routing
- Flow diversion and regulation
- Tidal/backwater effects at outfalls
- Surcharge and flooding simulation
- Culvert inlet control

#### Water Quality
- Pollutant buildup and washoff from surfaces
- Pollutant routing through the drainage network
- Treatment device simulation with user-defined removal functions
- First flush and other quality phenomena
- Multiple pollutant constituents

### 2.2 Primary Use Cases

1. **Urban Drainage Design**: Sizing of drainage system components
2. **Flood Risk Assessment**: Evaluation of flooding potential and mitigation strategies
3. **Combined Sewer Overflow (CSO) Analysis**: Assessment and control of CSO events
4. **Green Infrastructure Planning**: Evaluation of LID/Green Infrastructure practices
5. **Water Quality Management**: Pollutant load estimation and BMP effectiveness
6. **Long-term Continuous Simulation**: Climate change and watershed planning studies

## 3. System Architecture

### 3.1 Core Components

The SWMM computational engine consists of approximately 50,000 lines of C code organized into 55 source files plus 22 header files. The architecture follows a modular design with the following major components:

#### 3.1.1 Supervisory Control Layer
- **swmm5.c**: Main API providing supervisory control functions
- **project.c**: Project data management, object creation/destruction, ID lookup
- **input.c**: Input file parsing and data loading
- **output.c**: Binary output file operations
- **report.c**: Text report generation
- **error.c**: Error handling and reporting

#### 3.1.2 Runoff Computation Layer
- **runoff.c**: Main runoff quantity and quality computations
- **rain.c**: Rainfall data processing from external files
- **gage.c**: Rain gage data management
- **climate.c**: Temperature, evaporation, and wind speed data
- **snow.c**: Snow accumulation, removal, and melt processes
- **infil.c**: Infiltration calculations using various methods
- **gwater.c**: Groundwater flux computations
- **subcatch.c**: Subcatchment-level runoff and pollutant processes
- **landuse.c**: Land use-based pollutant buildup and washoff
- **lid.c**: Low Impact Development practice evaluation
- **lidproc.c**: Individual LID unit hydrologic performance

#### 3.1.3 Routing Computation Layer
- **routing.c**: Main control of network flow routing
- **flowrout.c**: Top-level flow routing control
- **kinwave.c**: Kinematic wave routing method
- **dynwave.c**: Dynamic wave routing method
- **dwflow.c**: Dynamic wave equation solver for individual conduits
- **node.c**: Node (junction, storage, divider, outfall) computations
- **link.c**: Link (conduit, pump, orifice, weir, outlet) computations
- **inflow.c**: Direct and dry weather inflows
- **rdii.c**: Rainfall-dependent infiltration/inflow
- **qualrout.c**: Water quality constituent routing
- **treatmnt.c**: Pollutant treatment and removal
- **controls.c**: Rule-based control implementation
- **inlet.c**: Street inlet flow capture (FHWA HEC-22 methods)
- **forcmain.c**: Force main friction losses
- **culvert.c**: Culvert inlet control

#### 3.1.4 Support and Utility Layer
- **hash.c**: Hash table implementation for fast object retrieval
- **table.c**: Lookup tables for curves and time series
- **mathexpr.c**: Mathematical expression parser and evaluator
- **datetime.c**: Date and time manipulation functions
- **findroot.c**: Root-finding algorithms
- **odesolve.c**: Fifth-order Runge-Kutta ODE solver
- **mempool.c**: Memory pool management
- **keywords.c**: Input file keyword definitions
- **shape.c**: User-defined conduit shape geometry
- **street.c**: Street cross-section geometry
- **transect.c**: Irregular channel cross-section geometry
- **xsect.c**: Conduit cross-section geometric properties
- **toposort.c**: Network topology sorting and cycle detection
- **hotstart.c**: System state saving and loading for hot start files

#### 3.1.5 Analysis and Reporting Layer
- **massbal.c**: Mass balance accounting for runoff and routing
- **stats.c**: Statistical data collection during simulation
- **statsrpt.c**: Summary statistics reporting
- **inputrpt.c**: Input data summary reporting

### 3.2 Data Model

SWMM organizes simulation data into well-defined object types:

#### Major Object Types
1. **GAGE**: Rain gages that supply rainfall data
2. **SUBCATCH**: Subcatchment areas that generate runoff
3. **NODE**: Conveyance system nodes (junctions, outfalls, storage, dividers)
4. **LINK**: Conveyance system links (conduits, pumps, orifices, weirs, outlets)
5. **POLLUT**: Pollutant constituents tracked through the system
6. **LANDUSE**: Land use categories for pollutant modeling
7. **TIMEPATTERN**: Temporal patterns for dry weather flows
8. **CURVE**: Generic lookup tables
9. **TSERIES**: Generic time series data
10. **CONTROL**: Rule-based control definitions
11. **TRANSECT**: Irregular channel cross-sections
12. **AQUIFER**: Groundwater aquifer parameters
13. **UNITHYD**: RDII unit hydrographs
14. **SNOWMELT**: Snow melt parameter sets
15. **SHAPE**: Custom conduit shapes
16. **LID**: Low Impact Development treatment units
17. **STREET**: Street cross-section definitions
18. **INLET**: Street inlet designs

### 3.3 Computational Flow

A typical SWMM simulation proceeds through the following phases:

1. **Initialization Phase**
   - Open and parse input file
   - Validate network connectivity and topology
   - Allocate memory for simulation objects
   - Initialize state variables
   - Open output files
   - Load hot start data if specified

2. **Runoff Phase** (for each time step)
   - Process rainfall data
   - Compute snowfall/snowmelt
   - Calculate infiltration losses
   - Determine surface runoff
   - Compute LID performance
   - Calculate pollutant buildup and washoff
   - Route groundwater

3. **Routing Phase** (for each routing time step)
   - Apply direct inflows and RDII
   - Execute control rules
   - Solve hydraulic equations (kinematic or dynamic wave)
   - Update node depths and link flows
   - Route water quality constituents
   - Apply treatment functions
   - Check for flooding and surcharge

4. **Reporting Phase**
   - Write results to output file
   - Update mass balance accounting
   - Collect statistics

5. **Finalization Phase**
   - Compute final mass balances
   - Write summary statistics
   - Generate report file
   - Save hot start data if specified
   - Close files and free memory

## 4. Application Programming Interface (API)

### 4.1 Core SWMM API

The main SWMM API provides basic simulation control:

- **swmm_open()**: Open and initialize a SWMM project
- **swmm_start()**: Start a simulation run
- **swmm_step()**: Advance simulation by one time step
- **swmm_end()**: End a simulation and write results
- **swmm_report()**: Generate detailed text report
- **swmm_close()**: Close project and free resources
- **swmm_getError()**: Retrieve error messages
- **swmm_getWarnings()**: Get warning count
- **swmm_getVersion()**: Get version information

### 4.2 Extended Toolkit API

The OWA community has developed an extensive toolkit API that provides programmatic access to SWMM's internal data structures and allows manipulation before, during, and after simulation. This includes over 80 additional functions organized by category:

#### Project and Object Management
- Object finding and counting
- ID retrieval and validation

#### Simulation Settings
- Date/time manipulation
- Analysis setting queries
- Parameter and unit retrieval

#### Node Operations
- Get/set node parameters
- Retrieve node results (depth, head, volume, inflow)
- Access pollutant concentrations
- Obtain node statistics (maximum depth, flooding duration, etc.)

#### Link Operations
- Get/set link parameters
- Retrieve link results (flow, velocity, depth)
- Access pollutant concentrations
- Control link settings (pump speed, gate opening)
- Obtain link statistics

#### Subcatchment Operations
- Get/set subcatchment parameters
- Retrieve runoff results
- Access pollutant loads
- Obtain subcatchment statistics

#### LID Operations
- Access LID unit counts, parameters, and options
- Retrieve LID flux rates and results
- Control LID operation

#### System-Level Operations
- Retrieve routing and runoff totals
- System-wide mass balance information

#### Rain Gage Operations
- Get/set precipitation rates
- Real-time rainfall modification

### 4.3 Output File API

A separate output library provides structured access to binary output files:
- Time series extraction for any object
- Metadata retrieval (object counts, variable types, units)
- Statistical summaries

## 5. Simulation Methods

### 5.1 Runoff Methods

#### Infiltration Models
1. **Horton Method**: Exponential decay of infiltration capacity
2. **Modified Horton Method**: Enhanced for continuous simulation
3. **Green-Ampt Method**: Physics-based infiltration model
4. **Modified Green-Ampt Method**: Improved redistribution phase
5. **Curve Number Method**: NRCS/SCS empirical method

#### Surface Routing
- Nonlinear reservoir routing for overland flow
- Manning's equation for flow velocity
- Depression storage and surface ponding

#### LID Types Supported
- Bio-retention cells
- Rain gardens
- Green roofs
- Infiltration trenches
- Porous pavement
- Rain barrels/cisterns
- Vegetative swales
- Rooftop disconnection

### 5.2 Hydraulic Routing Methods

#### Steady Flow Routing
- Uniform flow equations using Manning's formula
- Suitable for preliminary studies

#### Kinematic Wave Routing
- Solves continuity equation with simplified momentum equation
- Assumes flow is a function of depth only
- Computationally efficient
- Suitable for steep systems with mild subcritical flow

#### Dynamic Wave Routing
- Full solution of Saint-Venant equations
- Solves continuity and momentum equations
- Handles backwater effects, flow reversals, pressurization
- Most accurate but computationally intensive
- Required for complex systems with significant storage, backwater, or surcharge

### 5.3 Water Quality Simulation

#### Buildup and Washoff Models
- Power function buildup
- Exponential buildup
- Saturation buildup
- External time series buildup
- Exponential washoff
- Rating curve washoff
- Event Mean Concentration (EMC) washoff

#### Routing Methods
- Plug flow routing
- Complete mixing in storage nodes
- Treatment functions with user-defined removal expressions

## 6. Input and Output

### 6.1 Input File Format

SWMM uses a structured text input file with clearly defined sections denoted by bracketed keywords:

```
[OPTIONS]
[RAINGAGES]
[SUBCATCHMENTS]
[SUBAREAS]
[INFILTRATION]
[JUNCTIONS]
[OUTFALLS]
[STORAGE]
[CONDUITS]
[PUMPS]
[ORIFICES]
[WEIRS]
[OUTLETS]
[POLLUTANTS]
[LANDUSES]
[TIMESERIES]
[CURVES]
[CONTROLS]
... and many more sections
```

Each section contains data in tabular format with space or tab-delimited columns.

### 6.2 Output Files

#### Status Report (.rpt)
- Text file containing:
  - Summary of input data
  - Continuity errors
  - Flow routing statistics
  - Node and link summaries
  - Optionally detailed time series results

#### Binary Output File (.out)
- Compact binary format storing:
  - Time series results for all objects
  - Sampling interval and reporting periods
  - Object and variable metadata
- Accessed through Output File API

#### Interface Files
- Rainfall interface files
- Runoff interface files
- Hot start files (system state)
- RDII interface files
- Inflows/outflows interface files

## 7. Technical Specifications

### 7.1 Computational Characteristics

- **Time Step**: Variable (adaptive) for dynamic wave routing; user-specified for other methods
- **Minimum Time Step**: Typically 0.5 to 1 second for dynamic wave
- **Maximum Conduits**: Limited only by available memory (thousands supported)
- **Solution Method**: Iterative Newton-Raphson for dynamic wave
- **Convergence Criteria**: User-specified tolerances for continuity and flow

### 7.2 Numerical Methods

- **ODE Solver**: Fifth-order Runge-Kutta method (RK5) for LID processes
- **Matrix Solver**: Sparse matrix techniques for dynamic wave
- **Root Finding**: Modified bisection and Newton methods
- **Integration**: Euler method with variable time steps

### 7.3 Performance Considerations

- **Parallelization**: OpenMP support for multi-threaded execution
- **Memory Management**: Efficient object pooling and hash tables
- **File I/O**: Buffered binary I/O for output files
- **Optimization**: Platform-specific compiler optimizations supported

### 7.4 Platform Support

- **Operating Systems**: Windows, Linux, macOS
- **Compilers**: MSVC, GCC, MinGW, Clang
- **Architecture**: x86, x64, ARM (with appropriate compiler)
- **Dependencies**: Minimal (standard C library only for core engine)

## 8. Testing and Quality Assurance

### 8.1 Testing Framework

- **Unit Tests**: Component-level testing using Boost test framework
- **Regression Tests**: Extensive suite comparing results against benchmark datasets
- **Continuous Integration**: Automated build and test on GitHub Actions
- **Platform Testing**: Multi-platform testing (Windows, Linux, macOS)

### 8.2 Validation

The SWMM model has been extensively validated through:
- Academic research studies
- Real-world case studies
- Comparison with measured data from instrumented systems
- Peer-reviewed publications spanning decades

## 9. Integration and Extensibility

### 9.1 Integration Points

SWMM is designed as a computational engine that can be embedded in larger applications:

- **GUI Applications**: SWMM GUI, PCSWMM, and others
- **Python Integration**: PySWMM wrapper library
- **R Integration**: swmmr package
- **Web Services**: RESTful APIs wrapping SWMM engine
- **GIS Integration**: ArcGIS, QGIS plugins

### 9.2 Extensibility

The modular architecture allows for:
- Custom hydraulic elements (pumps, weirs, etc.)
- User-defined treatment functions
- External rainfall data sources
- Custom control rules
- Third-party output post-processors

## 10. Development and Contribution

### 10.1 Development Environment

- **Source Control**: Git/GitHub
- **Build System**: CMake cross-platform build
- **Documentation**: Doxygen for API documentation
- **Code Style**: Consistent C coding conventions
- **Issue Tracking**: GitHub Issues

### 10.2 Contributing

The project welcomes contributions:
- Bug reports and feature requests via GitHub Issues
- Code contributions via Pull Requests
- Testing and validation of new features
- Documentation improvements
- Community support and discussion

### 10.3 Governance

The project is governed by the Open Water Analytics community with:
- Collaborative decision-making
- Code review process
- Contributor Covenant Code of Conduct
- Regular release cycle with semantic versioning

## 11. Licensing and Citation

### 11.1 License

SWMM is released under a dual license:
- **Public Domain**: Legacy EPA code
- **MIT License**: OWA community contributions

This ensures maximum flexibility for both academic and commercial use.

### 11.2 Citation

Users are encouraged to cite SWMM in publications:

**DOI**: 10.5281/zenodo.5484299

**Recommended Citation**:
McDonnell, B., Wu, J. X., Ratliff, K., Mullapudi, A., & Tryby, M. (2021). 
Open Water Analytics Stormwater Management Model (Version 5.1.13). 
Zenodo. https://doi.org/10.5281/zenodo.5484299

## 12. Resources and Support

### 12.1 Documentation

- **GitHub Repository**: https://github.com/OpenWaterAnalytics/Stormwater-Management-Model
- **API Documentation**: http://wateranalytics.org/Stormwater-Management-Model/
- **EPA SWMM Website**: https://www.epa.gov/water-research/storm-water-management-model-swmm
- **User Manual**: Available from EPA website

### 12.2 Community

- **GitHub Discussions**: Feature discussions and questions
- **GitHub Issues**: Bug reports and feature requests
- **Mailing Lists**: Community discussions
- **Stack Overflow**: Tagged swmm questions

### 12.3 Related Projects

- **PySWMM**: Python wrapper for real-time control and analysis
- **swmmr**: R package for SWMM interaction
- **SWMM Output Toolkit**: Enhanced output file processing
- **Open Storm**: Water resources modeling platform

## 13. Future Directions

The SWMM project continues to evolve with planned enhancements:

- Enhanced parallel computing capabilities
- Additional LID practice types
- Improved calibration and uncertainty analysis tools
- Cloud-based execution and scaling
- Real-time control optimization
- Climate change adaptation features
- Enhanced water quality modeling
- Better integration with modern GIS platforms

## 14. Conclusion

SWMM represents a mature, robust, and actively maintained computational engine for urban drainage and stormwater management modeling. Its open-source nature, comprehensive capabilities, and active community make it a valuable tool for researchers, engineers, and planners worldwide. The combination of physical-based modeling, extensive validation, and flexible architecture ensures SWMM will remain a cornerstone of urban water management analysis for years to come.

---

**Document Version**: 1.0  
**Date**: February 2026  
**Based on**: SWMM Version 5.2.4  
**Repository**: https://github.com/OpenWaterAnalytics/Stormwater-Management-Model
