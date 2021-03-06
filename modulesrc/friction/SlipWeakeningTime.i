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

/** @file modulesrc/friction/SlipWeakeningTime.i
 *
 * Python interface to C++ SlipWeakeningTime object.
 */

namespace pylith {
  namespace friction {

    class SlipWeakeningTime : public FrictionModel
    { // class SlipWeakeningTime

      // PUBLIC METHODS /////////////////////////////////////////////////
    public :

      /// Default constructor.
      SlipWeakeningTime(void);

      /// Destructor.
      ~SlipWeakeningTime(void);

      // PROTECTED METHODS //////////////////////////////////////////////
    protected :

      /** Compute properties from values in spatial database.
       *
       * @param propValues Array of property values.
       * @param dbValues Array of database values.
       */
      void _dbToProperties(PylithScalar* const propValues,
			   const scalar_array& dbValues) const;

      /** Nondimensionalize properties.
       *
       * @param values Array of property values.
       * @param nvalues Number of values.
       */
      void _nondimProperties(PylithScalar* const values,
			     const int nvalues) const;

      /** Dimensionalize properties.
       *
       * @param values Array of property values.
       * @param nvalues Number of values.
       */
      void _dimProperties(PylithScalar* const values,
			  const int nvalues) const;

      /** Compute friction from properties and state variables.
       *
       * @param t Current time.
       * @param slip Current slip at location.
       * @param slipRate Current slip rate at location.
       * @param normalTraction Normal traction at location.
       * @param properties Properties at location.
       * @param numProperties Number of properties.
       * @param stateVars State variables at location.
       * @param numStateVars Number of state variables.
       */
      PylithScalar _calcFriction(const PylithScalar t,
				 const PylithScalar slip,
				 const PylithScalar slipRate,
				 const PylithScalar normalTraction,
				 const PylithScalar* properties,
				 const int numProperties,
				 const PylithScalar* stateVars,
				 const int numStateVars);

      /** Compute derivative of friction with slip from properties and
       * state variables.
       *
       * @param t Time in simulation.
       * @param slip Current slip at location.
       * @param slipRate Current slip rate at location.
       * @param normalTraction Normal traction at location.
       * @param properties Properties at location.
       * @param numProperties Number of properties.
       * @param stateVars State variables at location.
       * @param numStateVars Number of state variables.
       *
       * @returns Derivative of friction (magnitude of shear traction) at vertex.
       */
      PylithScalar _calcFrictionDeriv(const PylithScalar t,
				      const PylithScalar slip,
				      const PylithScalar slipRate,
				      const PylithScalar normalTraction,
				      const PylithScalar* properties,
				      const int numProperties,
				      const PylithScalar* stateVars,
				      const int numStateVars);

    }; // class SlipWeakeningTime

  } // friction
} // pylith


// End of file 
