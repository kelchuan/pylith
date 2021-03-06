// -*- C++ -*-
//
// ----------------------------------------------------------------------
//
// Brad T. Aagaard, U.S. Geological Survey
// Charles A. Williams, GNS Science
// Matthew G. Knepley, University of Chicago
//
// This code was developed as part of the Computational Infrastructure
// for Geodynamics (http://geodynamics.org).
//
// Copyright (c) 2010-2017 University of California, Davis
//
// See COPYING for license information.
//
// ----------------------------------------------------------------------
//

#include <portinfo>

#include "SlipWeakeningTimeStable.hh" // implementation of object methods

#include "pylith/materials/Metadata.hh" // USES Metadata

#include "pylith/utils/array.hh" // USES scalar_array
#include "pylith/utils/constdefs.h" // USES MAXSCALAR

#include "spatialdata/units/Nondimensional.hh" // USES Nondimensional

#include "petsc.h" // USES PetscLogFlops

#include <cassert> // USES assert()
#include <sstream> // USES std::ostringstream
#include <stdexcept> // USES std::runtime_error
// ----------------------------------------------------------------------
namespace pylith {
  namespace friction {
    namespace _SlipWeakeningTimeStable {

      // Number of physical properties.
      const int numProperties = 6;

      // Physical properties.
      const pylith::materials::Metadata::ParamDescription properties[6] = {
	{ "static_coefficient", 1, pylith::topology::FieldBase::SCALAR },
	{ "dynamic_coefficient", 1, pylith::topology::FieldBase::SCALAR },
        { "slip_weakening_parameter", 1, pylith::topology::FieldBase::SCALAR },
	{ "cohesion", 1, pylith::topology::FieldBase::SCALAR },
	{ "time_weakening_time", 1, pylith::topology::FieldBase::SCALAR },
	{ "time_weakening_parameter", 1, pylith::topology::FieldBase::SCALAR },
      };

      // Number of State Variables.
      const int numStateVars = 2;

      // State Variables.
      const pylith::materials::Metadata::ParamDescription stateVars[2] = {
	{ "cumulative_slip", 1, pylith::topology::FieldBase::SCALAR },
	{ "previous_slip", 1, pylith::topology::FieldBase::SCALAR },
      };

      // Values expected in spatial database
      const int numDBProperties = 6;
      const char* dbProperties[6] = { "static-coefficient",
				      "dynamic-coefficient",
				      "slip-weakening-parameter",
				      "cohesion",
				      "time-weakening-time",
				      "time-weakening-parameter",
      };

      const int numDBStateVars = 2;
      const char* dbStateVars[2] = { "cumulative-slip",
				     "previous-slip",
      };      
      
    } // _SlipWeakeningTimeStable
  } // friction
} // pylith

// Indices of physical properties
const int pylith::friction::SlipWeakeningTimeStable::p_coefS = 0;
const int pylith::friction::SlipWeakeningTimeStable::p_coefD = 
  pylith::friction::SlipWeakeningTimeStable::p_coefS + 1;
const int pylith::friction::SlipWeakeningTimeStable::p_d0 = 
  pylith::friction::SlipWeakeningTimeStable::p_coefD + 1;
const int pylith::friction::SlipWeakeningTimeStable::p_cohesion =
  pylith::friction::SlipWeakeningTimeStable::p_d0 + 1;
const int pylith::friction::SlipWeakeningTimeStable::p_weaktime =
  pylith::friction::SlipWeakeningTimeStable::p_cohesion + 1;
const int pylith::friction::SlipWeakeningTimeStable::p_t0 =
  pylith::friction::SlipWeakeningTimeStable::p_weaktime + 1;

// Indices of database values (order must match dbProperties)
const int pylith::friction::SlipWeakeningTimeStable::db_coefS = 0;
const int pylith::friction::SlipWeakeningTimeStable::db_coefD = 
  pylith::friction::SlipWeakeningTimeStable::db_coefS + 1;
const int pylith::friction::SlipWeakeningTimeStable::db_d0 = 
  pylith::friction::SlipWeakeningTimeStable::db_coefD + 1;
const int pylith::friction::SlipWeakeningTimeStable::db_cohesion =
  pylith::friction::SlipWeakeningTimeStable::db_d0 + 1;
const int pylith::friction::SlipWeakeningTimeStable::db_weaktime =
  pylith::friction::SlipWeakeningTimeStable::db_cohesion + 1;
const int pylith::friction::SlipWeakeningTimeStable::db_t0 =
  pylith::friction::SlipWeakeningTimeStable::db_weaktime + 1;

// Indices of state variables.
const int pylith::friction::SlipWeakeningTimeStable::s_slipCum = 0;
const int pylith::friction::SlipWeakeningTimeStable::s_slipPrev = 
  pylith::friction::SlipWeakeningTimeStable::s_slipCum + 1;

// Indices of database values (order must match dbProperties)
const int pylith::friction::SlipWeakeningTimeStable::db_slipCum = 0;
const int pylith::friction::SlipWeakeningTimeStable::db_slipPrev = 
  pylith::friction::SlipWeakeningTimeStable::db_slipCum + 1;

// ----------------------------------------------------------------------
// Default constructor.
pylith::friction::SlipWeakeningTimeStable::SlipWeakeningTimeStable(void) :
  FrictionModel(materials::Metadata(_SlipWeakeningTimeStable::properties,
				    _SlipWeakeningTimeStable::numProperties,
				    _SlipWeakeningTimeStable::dbProperties,
				    _SlipWeakeningTimeStable::numDBProperties,
				    _SlipWeakeningTimeStable::stateVars,
				    _SlipWeakeningTimeStable::numStateVars,
				    _SlipWeakeningTimeStable::dbStateVars,
				    _SlipWeakeningTimeStable::numDBStateVars))
{ // constructor
} // constructor

// ----------------------------------------------------------------------
// Destructor.
pylith::friction::SlipWeakeningTimeStable::~SlipWeakeningTimeStable(void)
{ // destructor
} // destructor

// ----------------------------------------------------------------------
// Compute properties from values in spatial database.
void
pylith::friction::SlipWeakeningTimeStable::_dbToProperties(PylithScalar* const propValues,
							   const scalar_array& dbValues) const
{ // _dbToProperties
  assert(propValues);
  const int numDBValues = dbValues.size();
  assert(_SlipWeakeningTimeStable::numDBProperties == numDBValues);

  const PylithScalar db_static = dbValues[db_coefS];
  const PylithScalar db_dynamic = dbValues[db_coefD];
  const PylithScalar db_do = dbValues[db_d0];
  const PylithScalar db_c = dbValues[db_cohesion];
  const PylithScalar db_tw = dbValues[db_weaktime];
  const PylithScalar db_tp = dbValues[db_t0];

  if (db_static < 0.0) {
    std::ostringstream msg;
    msg << "Spatial database returned negative value for static coefficient "
	<< "of friction.\n"
	<< "static coefficient of friction: " << db_static << "\n";
    throw std::runtime_error(msg.str());
  } // if

  if (db_dynamic < 0.0) {
    std::ostringstream msg;
    msg << "Spatial database returned negative value for dynamic coefficient "
	<< "of friction.\n"
	<< "dynamic coefficient of friction: " << db_dynamic << "\n";
    throw std::runtime_error(msg.str());
  } // if

  if (db_d0 <= 0.0) {
    std::ostringstream msg;
    msg << "Spatial database returned nonpositive value for slip weakening parameter.\n"
	<< "slip weakening parameter: " << db_d0 << "\n";
    throw std::runtime_error(msg.str());
  } // if

  if (db_tw < 0.0) {
    std::ostringstream msg;
    msg << "Spatial database returned negative value for time-weakening time.\n"
	<< "weakening time: " << db_tw << "\n";
    throw std::runtime_error(msg.str());
  } // if

  if (db_t0 <= 0.0) {
    std::ostringstream msg;
    msg << "Spatial database returned nonpositive value for time-weakening paramter.\n"
	<< "time-weakening parameter: " << db_t0 << "\n";
    throw std::runtime_error(msg.str());
  } // if

  propValues[p_coefS] = db_static;
  propValues[p_coefD] = db_dynamic;
  propValues[p_d0] = db_do;
  propValues[p_cohesion] = db_c;
  propValues[p_weaktime] = db_tw;
  propValues[p_t0] = db_tp;

} // _dbToProperties

// ----------------------------------------------------------------------
// Nondimensionalize properties.
void
pylith::friction::SlipWeakeningTimeStable::_nondimProperties(PylithScalar* const values,
							     const int nvalues) const
{ // _nondimProperties
  assert(_normalizer);
  assert(values);
  assert(nvalues == _SlipWeakeningTimeStable::numProperties);

  const PylithScalar lengthScale = _normalizer->lengthScale();
  const PylithScalar pressureScale = _normalizer->pressureScale();
  const PylithScalar timeScale = _normalizer->timeScale();

  values[p_d0] /= lengthScale;
  values[p_cohesion] /= pressureScale;
  values[p_weaktime] /= timeScale;
  values[p_t0] /= timeScale;
} // _nondimProperties

// ----------------------------------------------------------------------
// Dimensionalize properties.
void
pylith::friction::SlipWeakeningTimeStable::_dimProperties(PylithScalar* const values,
							  const int nvalues) const
{ // _dimProperties
  assert(_normalizer);
  assert(values);
  assert(nvalues == _SlipWeakeningTimeStable::numProperties);

  const PylithScalar lengthScale = _normalizer->lengthScale();
  const PylithScalar pressureScale = _normalizer->pressureScale();
  const PylithScalar timeScale = _normalizer->timeScale();

  values[p_d0] *= lengthScale;
  values[p_cohesion] *= pressureScale;
  values[p_weaktime] *= timeScale;
  values[p_t0] *= timeScale;
} // _dimProperties

// ----------------------------------------------------------------------
// Compute state variables from values in spatial database.
void
pylith::friction::SlipWeakeningTimeStable::_dbToStateVars(PylithScalar* const stateValues,
							  const scalar_array& dbValues) const
{ // _dbToStateVars
  assert(stateValues);
  const int numDBValues = dbValues.size();
  assert(_SlipWeakeningTimeStable::numDBStateVars == numDBValues);

  const PylithScalar cumulativeSlip = dbValues[db_slipCum];
  const PylithScalar previousSlip = dbValues[db_slipPrev];
 
  stateValues[s_slipCum] = cumulativeSlip;
  stateValues[s_slipPrev] = previousSlip;
} // _dbToStateVars

// ----------------------------------------------------------------------
// Nondimensionalize state variables.
void
pylith::friction::SlipWeakeningTimeStable::_nondimStateVars(PylithScalar* const values,
							    const int nvalues) const
{ // _nondimStateVars
  assert(_normalizer);
  assert(values);
  assert(nvalues == _SlipWeakeningTimeStable::numStateVars);

  const PylithScalar lengthScale = _normalizer->lengthScale();

  values[s_slipCum] /= lengthScale;
  values[s_slipPrev] /= lengthScale;
} // _nondimStateVars

// ----------------------------------------------------------------------
// Dimensionalize state variables.
void
pylith::friction::SlipWeakeningTimeStable::_dimStateVars(PylithScalar* const values,
							 const int nvalues) const
{ // _dimStateVars
  assert(_normalizer);
  assert(values);
  assert(nvalues == _SlipWeakeningTimeStable::numStateVars);

  const PylithScalar lengthScale = _normalizer->lengthScale();

  values[s_slipCum] *= lengthScale;
  values[s_slipPrev] *= lengthScale;
} // _dimStateVars

// ----------------------------------------------------------------------
// Compute friction from properties and state variables.
PylithScalar
pylith::friction::SlipWeakeningTimeStable::_calcFriction(const PylithScalar t,
							 const PylithScalar slip,
							 const PylithScalar slipRate,
							 const PylithScalar normalTraction,
							 const PylithScalar* properties,
							 const int numProperties,
							 const PylithScalar* stateVars,
							 const int numStateVars)
{ // _calcFriction
  assert(properties);
  assert(_SlipWeakeningTimeStable::numProperties == numProperties);
  assert(stateVars);
  assert(_SlipWeakeningTimeStable::numStateVars == numStateVars);

  PylithScalar friction = 0.0;
  PylithScalar mu_f = 0.0;
  if (normalTraction <= 0.0) {
    // if fault is in compression
    const PylithScalar slipPrev = stateVars[s_slipPrev];
    const PylithScalar slipCum = stateVars[s_slipCum] + fabs(slip - slipPrev);

    const PylithScalar slipWeak = (slipCum >= properties[p_d0]) ? 1.0 : slipCum / properties[p_d0];
    PylithScalar timeWeak = 1.0;
    if (t < properties[p_weaktime]) {
      timeWeak = 0.0;
    } else if (t < properties[p_weaktime] + properties[p_t0]) {
      timeWeak = (t - properties[p_weaktime]) / properties[p_t0];
    } // if/else
    mu_f = properties[p_coefS] - (properties[p_coefS] - properties[p_coefD]) * std::max(slipWeak, timeWeak);
    friction = - mu_f * normalTraction + properties[p_cohesion];
  } // if

  PetscLogFlops(6);

  return friction;
} // _calcFriction


// ----------------------------------------------------------------------
// Compute derivative of friction with slip from properties and
// state variables.
PylithScalar
pylith::friction::SlipWeakeningTimeStable::_calcFrictionDeriv(const PylithScalar t,
							      const PylithScalar slip,
							      const PylithScalar slipRate,
							      const PylithScalar normalTraction,
							      const PylithScalar* properties,
							      const int numProperties,
							      const PylithScalar* stateVars,
							      const int numStateVars)
{ // _calcFrictionDeriv
  assert(properties);
  assert(_SlipWeakeningTimeStable::numProperties == numProperties);
  assert(stateVars);
  assert(_SlipWeakeningTimeStable::numStateVars == numStateVars);

  PylithScalar frictionDeriv = 0.0;
  if (normalTraction <= 0.0) {
    // if fault is in compression
    const PylithScalar slipPrev = stateVars[s_slipPrev];
    const PylithScalar slipCum = stateVars[s_slipCum] + fabs(slip - slipPrev);

    const PylithScalar slipWeak = (slipCum >= properties[p_d0]) ? 1.0 : slipCum / properties[p_d0];
    PylithScalar timeWeak = 1.0;
    if (t < properties[p_weaktime]) {
      timeWeak = 0.0;
    } else if (t < properties[p_weaktime] + properties[p_t0]) {
      timeWeak = (t - properties[p_weaktime]) / properties[p_t0];
    } // if/else
    if (slipWeak > timeWeak && slipCum < properties[p_d0]) {
      frictionDeriv = normalTraction * (properties[p_coefS] - properties[p_coefD]) / properties[p_d0];
    } // if
  } // if

  PetscLogFlops(10);

  return frictionDeriv;
} // _calcFrictionDeriv


// ----------------------------------------------------------------------
// Update state variables (for next time step).
void
pylith::friction::SlipWeakeningTimeStable::_updateStateVars(const PylithScalar t,
							    const PylithScalar slip,
							    const PylithScalar slipRate,
							    const PylithScalar normalTraction,
							    PylithScalar* const stateVars,
							    const int numStateVars,
							    const PylithScalar* properties,
							    const int numProperties)
{ // _updateStateVars
  assert(properties);
  assert(_SlipWeakeningTimeStable::numProperties == numProperties);
  assert(stateVars);
  assert(_SlipWeakeningTimeStable::numStateVars == numStateVars);

  const PylithScalar tolerance = 1.0e-12;
  if (slipRate > tolerance) {
    const PylithScalar slipPrev = stateVars[s_slipPrev];

    stateVars[s_slipPrev] = slip;
    stateVars[s_slipCum] += fabs(slip - slipPrev);
  } else {
    // Sliding has stopped, so reset state variables.
    stateVars[s_slipPrev] = slip;
    stateVars[s_slipCum] = 0.0;
  } // else
} // _updateStateVars


// End of file 
