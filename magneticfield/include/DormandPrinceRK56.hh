#include "G4MagIntegratorStepper.hh"

/*
 * DormandPrinceRK56.hh
 *
 *  Created on: 26-Jun-2015
 *      Author: hackabot
 */

#ifndef DORMAND_PRINCE_RK56_H
#define DORMAND_PRINCE_RK56_H

#include "G4MagIntegratorStepper.hh"

class DormandPrinceRK56 : public G4MagIntegratorStepper{
    
    
public:
    //constructor
    DormandPrinceRK56( G4EquationOfMotion *EqRhs,
               G4int numberOfVariables = 6,
               G4bool primary= true ) ;
    
    //destructor
    ~DormandPrinceRK56() ;
    
    //Stepper
    void Stepper( const G4double y[],
                 const G4double dydx[],
                 G4double h,
                 G4double yout[],
                 G4double yerr[] ) ;
 
//	No interpolation as of now :-
//    void interpolate( const G4double yInput[],
//                     const G4double dydx[],
//                     G4double yOut[],
//                     G4double Step,
//                     G4double tau
//                     ) ;
    

    
    G4double  DistChord()   const;
    G4int IntegratorOrder() const { return 5; }
//    G4bool isFSAL() const{ return false; }
    //    G4double *getLastDydx() {return 0;};
    
    DormandPrinceRK56(const DormandPrinceRK56&);
    DormandPrinceRK56& operator=(const DormandPrinceRK56&);
    
    
private:
    
	   G4double *ak2, *ak3, *ak4, *ak5, *ak6, *ak7, *ak8, *ak9,       // for storing intermediate 'k' values in stepper
    *yTemp, *yIn;
    
    G4double fLastStepLength;
    G4double *fLastInitialVector, *fLastFinalVector,
    *fLastDyDx, *fMidVector, *fMidError;
    // for DistChord calculations
    
    DormandPrinceRK56* fAuxStepper;
    
    //	G4int No_of_vars;
    //	G4double hinit, tinit, tmax, *yinit;
    //	double hmax, hmin, safe_const, err0, Step_factor;
    //	void (*derivs)(double, double *, double *);
    
    
};

#endif /* DormandPrinceRK56 */

