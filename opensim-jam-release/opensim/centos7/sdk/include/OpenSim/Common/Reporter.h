#ifndef OPENSIM_REPORTER_H_
#define OPENSIM_REPORTER_H_
/* -------------------------------------------------------------------------- *
 *                             OpenSim:  Reporter.h                           *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2017 Stanford University and the Authors                *
 * Author(s): Ajay Seth                                                       *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */
// INCLUDE
#include <OpenSim/Common/Component.h>
#include <OpenSim/Common/TimeSeriesTable.h>

namespace OpenSim {

/**
 * This abstract class represents a Reporter that generates a report from 
 * values generated by Components in a Model during a simulation. 
 *
 * The what, how and where values are reported (e.g. to the console,
 * DataTable, device, etc...) are the purview of concrete Reporters
 * (e.g., TableReporter).
 *
 * @note These reporter classes have *no* association with some of the
 * Analyses that have "reporter" in their name (like ForceReporter).
 *
 * @ingroup reporters
 *
 * @author Ajay Seth
 */
class OSIMCOMMON_API AbstractReporter : public Component {
OpenSim_DECLARE_ABSTRACT_OBJECT(AbstractReporter, Component);
public:
//==============================================================================
// PROPERTIES
//==============================================================================

    OpenSim_DECLARE_PROPERTY(report_time_interval, double,
        "The recording time interval (s). If interval == 0, defaults to"
        "every valid integration time step.");

//=============================================================================
// PUBLIC METHODS
//=============================================================================
    /** Report values given the state and top-level Component (e.g. Model) */
    void report(const SimTK::State& s) const;

protected:
    /** Default constructor sets up Reporter-level properties; can only be
    called from a derived class constructor. **/
    AbstractReporter();

    virtual ~AbstractReporter() = default;

    /** Enable de/serialization from/to XML for derived classes  **/
    AbstractReporter(SimTK::Xml::Element& node);

    //--------------------------------------------------------------------------
    // Reporter interface.
    //--------------------------------------------------------------------------
    virtual void implementReport(const SimTK::State& state) const = 0;

    //--------------------------------------------------------------------------
    // Component interface.
    //--------------------------------------------------------------------------
    /** Add reporter required resources to the underlying System */
    void extendAddToSystem(SimTK::MultibodySystem& system) const override;

    /** Extend the Reporting functionality. Only does anything if
     report_time_interval is 0.  */
    void extendRealizeReport(const SimTK::State& state) const override final;

private:
    void setNull();
    void constructProperties();

//=============================================================================
};  // END of class AbstractReporter
//=============================================================================

/**
* This is an abstract Reporter with a single list Input named "inputs" whose
* type is templated (InputT).
*
* Destination of reported values are left to concrete types.
*
* @ingroup reporters
*
* @author Ajay Seth
*/
template<typename InputT = SimTK::Real>
class Reporter : public AbstractReporter {
    OpenSim_DECLARE_ABSTRACT_OBJECT_T(Reporter, InputT, AbstractReporter);
public:
    //=========================================================================
    // INPUTS
    //=========================================================================
    OpenSim_DECLARE_LIST_INPUT(inputs, InputT, SimTK::Stage::Report,
        "Variable list of quantities to be reported.");

    //==========================================================================
    // PUBLIC METHODS
    //==========================================================================

    /** Connect an output (single-valued or list) to this reporter. 
    The output must be of type InputT.
    If the output is a list output, this connects to all of the channels of the
    output. You can optionally provide an alias that will be used by this
    component to refer to the output; the alias will be used for all channels
    of the output.                                          
    @code
    auto* reporter = new ConsoleReporter();
    auto* src = new TableSource();
    reporter->addToReport(src->getOutput("all_columns"));
    @endcode
    This method is equivalent to
    connectInput_inputs(const AbstractOutput&, const std::string&). */
    void addToReport(const AbstractOutput& output,
                     const std::string& alias = "") {
        connectInput_inputs(output, alias);
    }

    /** Connect an output channel to this reporter.
    The output channel must be of type InputT.
    You can optionally provide an alias that will be used by this component to
    refer to the channel.
    @code
    auto* reporter = new ConsoleReporter();
    auto* src = new TableSource();
    reporter->addToReport(src->getOutput("column").getChannel("ankle"));
    @endcode
    This method is equivalent to
    connectInput_inputs(const AbstractChannel&, const std::string&). */
    void addToReport(const AbstractChannel& channel,
                     const std::string& alias = "") {
        connectInput_inputs(channel, alias);
    }

protected:
    /** Default constructor sets up Reporter-level properties; can only be
    called from a derived class constructor. **/
    Reporter() = default;
    virtual ~Reporter() = default;
    //=============================================================================
};  // END of class Reporter<InputT>
    //=============================================================================


/**
* This concrete Reporter class collects and reports Output<InputT>s within a
* TimeSeriesTable_. The column labels in the resulting table come from the
* names of the outputs connected to this reporter. The contents of the table are
* the Output values with each row being the value of all outputs at subsequent
* times determined by the reporting interval.
*
* @ingroup reporters
*
* @tparam InputT The type for the Reporter's Input (i.e., Reporter<InputT>).
* @tparam ValueT The type for the TimeSeriesTable (i.e., TimeSeriesTable_<ValueT>).
*
* @author Ajay Seth
*/
template<class InputT=SimTK::Real, typename ValueT=InputT>
class TableReporter_ : public Reporter<InputT> {
OpenSim_DECLARE_CONCRETE_OBJECT_T(TableReporter_, InputT, Reporter<InputT>);
public:
    TableReporter_() = default;
    virtual ~TableReporter_() = default;

    /** Retrieve the report as a TimeSeriesTable.                             */
    const TimeSeriesTable_<ValueT>& getTable() const {
        return _outputTable;
    }

    /** Clear the report. This can be used for example in loops performing 
    simulation. Each new iteration should start with an empty report and so this
    function can be used to clear the report at the end of each iteration.    */
    void clearTable() {
        std::vector<std::string> columnLabels;
        // Handle the case where no outputs were connected to the reporter.
        if (_outputTable.hasColumnLabels()) {
            columnLabels = _outputTable.getColumnLabels();
        }
        _outputTable = TimeSeriesTable_<ValueT>{};
        if (!columnLabels.empty()) {
            _outputTable.setColumnLabels(columnLabels);
        }
    }

protected:
    void implementReport(const SimTK::State& state) const override {
        const auto& input = this->template getInput<InputT>("inputs");
        SimTK::RowVector_<ValueT> result;
        result.resize(int(input.getNumConnectees()));

        for (auto idx = 0u; idx < input.getNumConnectees(); ++idx) {
              const auto& chan = input.getChannel(idx);
              const auto& value = chan.getValue(state);
              result[idx] = value;
        }
        try {
            const_cast<Self*>(this)->_outputTable.appendRow(state.getTime(),
                                                            result);
        } catch(const InvalidTimestamp& exception) {
            OPENSIM_THROW(Exception,
                          "Attempting to update reporter with rows having "
                          "invalid timestamps. Hint: If running simulation in "
                          "a loop, use clearTable() to clear table at the end "
                          "of each loop.\n\n" + std::string{exception.what()});
        }
    }

    void extendFinalizeConnections(Component& root) override {
        Super::extendFinalizeConnections(root);

        const auto& input = this->template getInput<InputT>("inputs");

        std::vector<std::string> labels;
        for (auto idx = 0u; idx < input.getNumConnectees(); ++idx) {
            labels.push_back( input.getLabel(idx) );
        }
        if (!labels.empty()) {
            const_cast<Self*>(this)->_outputTable.setColumnLabels(labels);
        } else {
            std::cout << "Warning: No outputs were connected to '"
                      << this->getName() << "' of type "
                      << getConcreteClassName() << ". You can connect outputs "
                      "by calling addToReport()." << std::endl;

        }
    }

private:

    // Hold the output values in a table with values as columns and time rows
    // We write to this table in const methods, but only because we ensure
    // those const methods are never called with trial integrator states.
    TimeSeriesTable_<ValueT> _outputTable;
};

/** A reporter that simply prints quantities to the console
 (command window or terminal), perhaps to monitor the progress of a simulation 
 as it executes.
 @ingroup reporters
  */
template <typename T>
class ConsoleReporter_ : public Reporter<T> {
    OpenSim_DECLARE_CONCRETE_OBJECT_T(ConsoleReporter_, T, Reporter<T>);

    // TODO num significant digits (override).
public:
    ConsoleReporter_() = default;
    virtual ~ConsoleReporter_() = default;

private:
    void implementReport(const SimTK::State& state) const override {
        // Output::getNumberOfSignificantDigits().
        const auto& input = this->template getInput<T>("inputs");

        if (state.getTime() <= SimTK::Eps) {
            // reset print count if we reset the simulation
            const_cast<ConsoleReporter_<T>*>(this)->_printCount = 0;
        }

        // Periodically display column headers.
        if (_printCount % 40 == 0) {
            std::cout << "[" << this->getName() << "]" << "\n";

            // Split labels over multiple lines. First, find the length of the
            // longest label.
            int longestLabel = 0;
            for (auto idx = 0u; idx < input.getNumConnectees(); ++idx)
                longestLabel = std::max(longestLabel,
                                        (int)input.getLabel(idx).size());

            // Round up to the nearest multiple of _width to determine the
            // number of header rows.
            const int numHeaderRows = (longestLabel-1) / _width + 1;

            // Display labels in chunks of size _width.
            for (int row = 0; row < numHeaderRows; ++row)
            {
                // Time column.
                if (row == numHeaderRows-1)
                    std::cout << std::setw(_width) << "time" << "| ";
                else
                    std::cout << std::setw(_width+2) << "| ";

                // Data columns.
                for (auto idx = 0u; idx < input.getNumConnectees(); ++idx) {
                    const auto& outName = input.getLabel(idx);
                    const std::string lbl =
                        std::string(numHeaderRows*_width - outName.size(), ' ')
                        + outName;
                    std::cout << lbl.substr(_width*row, _width) << "| ";
                }
                std::cout << "\n";
            }

            // Horizontal rule.
            for (auto idx = 0u; idx <= input.getNumConnectees(); ++idx)
                std::cout << std::string(_width, '-') << "| ";
            std::cout << "\n";
        }

        // TODO set width based on number of significant digits.
        std::cout << std::setw(_width) << std::scientific
                  << state.getTime() << "| ";
        for (const auto& chan : input.getChannels()) {
            const auto& value = chan->getValue(state);
            const auto& nSigFigs = chan->getOutput().getNumberOfSignificantDigits();
            std::cout << std::setw(_width) << std::scientific
                      << std::setprecision(nSigFigs) << value << "| ";
        }
        std::cout << std::endl;

        const_cast<ConsoleReporter_<T>*>(this)->_printCount++;
    }

    unsigned int _printCount = 0;
    int _width = 14;
};

// specialization where InputT is Vector_<T> and ValueT is Real
template<>
inline void TableReporter_<SimTK::Vector, SimTK::Real>::
    implementReport(const SimTK::State& state) const
{
    const auto& input = getInput<SimTK::Vector>("inputs");
    const SimTK::Vector& result = input.getValue(state, 0);
    
    if (_outputTable.getNumRows() == 0) {
        std::vector<std::string> labels;
        const std::string& base = input.getLabel(0);
        for (int ix = 0; ix < result.size(); ++ix) {
            labels.push_back(base + "[" + std::to_string(ix)+"]");
        }
        const_cast<Self*>(this)->_outputTable.setColumnLabels(labels);
    }

    const_cast<Self*>(this)->_outputTable.appendRow(state.getTime(), 
                                                    (~result).getAsRowVector());
}

/** @name Commonly used concrete TableReporters */
/// @{
/** This table can report doubles, and is the most common reporter that you
would want to use. You can use this reporter to report muscle activation,
 a coordinate value, etc.
@relates TableReporter_
@ingroup reporters
*/
typedef TableReporter_<SimTK::Real> TableReporter;
/** This table can report SimTK::Vec3%s, and thus can be used for reporting
 positions, velocities, accelerations, etc.
@relates TableReporter_
@ingroup reporters
*/
typedef TableReporter_<SimTK::Vec3> TableReporterVec3;
/** This table can report SimTK::Vector%s, and thus can be used for reporting
vector control signals, or all the generalized coordinates in one vector.
@relates TableReporter_
@ingroup reporters
*/
typedef TableReporter_<SimTK::Vector, SimTK::Real> TableReporterVector;
/// @}

/** @name Commonly used concrete ConsoleReporters */
/// @{
/** This table can report doubles; you can use this reporter to report muscle
 activation, a coordinate value, etc.
@relates ConsoleReporter_
@ingroup reporters
*/
typedef ConsoleReporter_<SimTK::Real> ConsoleReporter;
/** This table can report SimTK::Vec3%s, and thus can be used for reporting
 positions, velocities, accelerations, etc.
@relates ConsoleReporter_
@ingroup reporters
*/
typedef ConsoleReporter_<SimTK::Vec3> ConsoleReporterVec3;
/// @}
    

//=============================================================================
} // end of namespace OpenSim

#endif // OPENSIM_REPORTER_H_


