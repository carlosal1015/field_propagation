#include "G4MagIntegratorDriver.hh"

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4GeometryTolerance.hh"
#include "G4FieldTrack.hh"
#include "Utils.hh"

#include <assert.h>


const G4double G4MagInt_Driver::max_stepping_increase = 5.0;
const G4double G4MagInt_Driver::max_stepping_decrease = 0.1;

//  The (default) maximum number of steps is Base
//  divided by the order of Stepper
//
const G4int  G4MagInt_Driver::fMaxStepBase = 250;  // Was 5000

using namespace magneticfield;


G4MagInt_Driver::G4MagInt_Driver(
    G4double hminimum,
    G4MagIntegratorStepper* pStepper,
    G4int numComponents,
    G4int statisticsVerbose)
  : fSmallestFraction(1e-12),
    fStatisticsVerboseLevel(statisticsVerbose),
    fNoTotalSteps(0),
    fNoBadSteps(0),
    fNoSmallSteps(0),
    fNoInitialSmallSteps(0), 
    fVerboseLevel(0)
{  
    assert(numComponents == pStepper->GetNumberOfVariables());
    RenewStepperAndAdjust(pStepper);
    fMinimumStep = hminimum;
    fMaxNoSteps = fMaxStepBase / pIntStepper->IntegratorOrder();
}


G4MagInt_Driver::~G4MagInt_Driver()
{ 
}


// Runge-Kutta driver with adaptive stepsize control. Integrate starting
// values at y_current over hstep x2 with accuracy eps.
// On output ystart is replaced by values at the end of the integration
// interval. RightHandSide is the right-hand side of ODE system.
// The source is similar to odeint routine from NRC p.721-722 .
G4bool G4MagInt_Driver::AccurateAdvance(
    G4FieldTrack& track,
    G4double hstep,
    G4double eps,
    G4double hinitial)
{
    G4bool succeeded = true;
    if (hstep == 0) {
        std::ostringstream message;
        message << "Proposed step is zero; hstep = " << hstep << " !";
        G4Exception("G4MagInt_Driver::AccurateAdvance()",
                  "GeomField1001", JustWarning, message);
        return succeeded;
    }
    if (hstep < 0) {
        std::ostringstream message;
        message << "Invalid run condition." << G4endl
                << "Proposed step is negative; hstep = " << hstep << "." << G4endl
                << "Requested step cannot be negative! Aborting event.";
        G4Exception("G4MagInt_Driver::AccurateAdvance()",
                    "GeomField0003", EventMustBeAborted, message);
        return false;
    }

    G4int nstp, no_warnings = 0;
    G4double hnext, hdid;


    G4double y[G4FieldTrack::ncompSVEC], dydx[G4FieldTrack::ncompSVEC];
    G4bool lastStepSucceeded;

    G4int  noFullIntegr = 0, noSmallIntegr = 0;
    static G4ThreadLocal G4int noGoodSteps = 0;  // Bad = chord > curve-len

    track.DumpToArray(y);

    G4double startCurveLength = track.GetCurveLength();
    G4double x1 = startCurveLength;
    G4double x2 = x1 + hstep;
    G4double x = x1;

    G4double h = hstep;
    if (hinitial > perMillion * hstep && hinitial < hstep) {
        h = hinitial;
    }

    G4bool lastStep = false;
    nstp = 1;

    do {
        G4ThreeVector StartPos(y[0], y[1], y[2]);

        pIntStepper->ComputeRightHandSide(y, dydx);
        ++fNoTotalSteps;

        // Perform the Integration
        if (h > fMinimumStep) {
            OneGoodStep(y, dydx, x, h, eps, hdid, hnext);
            lastStepSucceeded = (hdid == h);
        } else {
            G4double yError[G4FieldTrack::ncompSVEC];
            pIntStepper->Stepper(y, dydx, h, y, yError);
            G4double dyerr = relativeError(y, yError, h, eps);
            hdid = h;
            x += hdid;
            hnext = ComputeNewStepSize(dyerr, h);
            lastStepSucceeded = (dyerr <= 1);
        }

        lastStepSucceeded ? ++noFullIntegr : ++noSmallIntegr;

        G4ThreeVector EndPos(y[0], y[1], y[2]);

        // Check the endpoint
        G4double endPointDist = (EndPos-StartPos).mag();
        if (endPointDist >= hdid * (1. + perMillion)) {
            ++fNoBadSteps;

            // Issue a warning only for gross differences -
            // we understand how small difference occur.
            if (endPointDist >= hdid * (1. + perThousand)){
               ++no_warnings;
            }
        } else {
           ++noGoodSteps;
        }

        //  Avoid numerous small last steps
        if ( (h < eps * hstep) || (h < fSmallestFraction * startCurveLength) ) {
            // No more integration -- the next step will not happen
            lastStep = true;
        } else {
            h = std::max(hnext, fMinimumStep);

            //  Ensure that the next step does not overshoot
            if (x + h > x2) {                // When stepsize overshoots, decrease it!
                h = x2 - x;   // Must cope with difficult rounding-error
            }                // issues if hstep << x2

            if (h == 0) {
                // Cannot progress - accept this as last step - by default
                lastStep = true;
            }
        }
    } while ( ((nstp++) <= fMaxNoSteps) && (x < x2) && (!lastStep) );
  // Loop checking, 07.10.2016, J. Apostolakis

     // Have we reached the end ?
     // --> a better test might be x-x2 > an_epsilon

    succeeded = (x >= x2);  // If it was a "forced" last step


    // Put back the values.
    track.LoadFromArray(y, pIntStepper->GetNumberOfVariables());
    track.SetCurveLength(x);

    if (nstp > fMaxNoSteps) {
        ++no_warnings;
        succeeded = false;
    }

    return succeeded;
}

// Step failed; compute the size of retrial Step.
G4double G4MagInt_Driver::ShrinkStepSize(G4double h, G4double error) const
{
    G4double htemp = GetSafety() * h * std::pow(error, GetPshrnk());
    return std::max(htemp, max_stepping_decrease * h);
}

// Compute size of next Step
G4double G4MagInt_Driver::GrowStepSize(G4double h, G4double error) const
{
    if (error > errcon) {
        return GetSafety() * h * std::pow(error, GetPgrow());
    }
    return max_stepping_increase * h;
}

// Driver for one Runge-Kutta Step with monitoring of local truncation error
// to ensure accuracy and adjust stepsize. Input are dependent variable
// array y[0,...,5] and its derivative dydx[0,...,5] at the
// starting value of the independent variable x . Also input are stepsize
// to be attempted htry, and the required accuracy eps. On output y and x
// are replaced by their new values, hdid is the stepsize that was actually
// accomplished, and hnext is the estimated next stepsize.
// This is similar to the function rkqs from the book:
// Numerical Recipes in C: The Art of Scientific Computing (NRC), Second
// Edition, by William H. Press, Saul A. Teukolsky, William T.
// Vetterling, and Brian P. Flannery (Cambridge University Press 1992),
// 16.2 Adaptive StepSize Control for Runge-Kutta, p. 719

void G4MagInt_Driver::OneGoodStep(
    G4double y[],
    const G4double dydx[],
    G4double& curveLength,   // InOut
    G4double htry,
    G4double eps_rel_max,
    G4double& hdid,          // Out
    G4double& hnext)         // Out
{
    G4double error;

    G4double yerror[G4FieldTrack::ncompSVEC], ytemp[G4FieldTrack::ncompSVEC];

    // Set stepsize to the initial trial value
    G4double hstep = htry;

    static G4ThreadLocal G4int tot_no_trials = 0;
    const G4int max_trials = 100;

    for (G4int iter = 0; iter < max_trials; ++iter) {
        ++tot_no_trials;

        pIntStepper-> Stepper(y, dydx, hstep, ytemp, yerror);
        error = relativeError(y, yerror, hstep, eps_rel_max);

        // Step succeeded.
        if (error <= 1) {
            break;
        }

        hstep = ShrinkStepSize(hstep, error);
    }

    hnext = GrowStepSize(hstep, error);
    curveLength += (hdid = hstep);

    for(G4int k = 0; k < pIntStepper->GetNumberOfVariables(); ++k) {
        y[k] = ytemp[k];
    }
}

G4bool G4MagInt_Driver::QuickAdvance(
    G4FieldTrack& fieldTrack,
    const G4double dydx[],
    G4double hstep,
    G4double& dchord_step,
    G4double& dyerr)
{
    G4double yError[G4FieldTrack::ncompSVEC],
             yIn[G4FieldTrack::ncompSVEC],
             yOut[G4FieldTrack::ncompSVEC];

    static G4ThreadLocal G4int no_call = 0;
    ++no_call;

    fieldTrack.DumpToArray(yIn);
    G4double s_start = fieldTrack.GetCurveLength();

    pIntStepper->Stepper(yIn, dydx, hstep, yOut, yError);
    dchord_step = pIntStepper->DistChord();

    // Put back the values.  yarrout ==> y_posvel
    fieldTrack.LoadFromArray(yOut, pIntStepper->GetNumberOfVariables());
    fieldTrack.SetCurveLength(s_start + hstep);


    dyerr = relativeError(yOut, yError, hstep);

    return true;
}

// This method computes new step sizes - but does not limit changes to
// within  certain factors
G4double G4MagInt_Driver::ComputeNewStepSize( 
    G4double  errMaxNorm,    // max error  (normalised)
    G4double  hstepCurrent)  // current step size
{
    if (errMaxNorm > 1) {
        return ShrinkStepSize(hstepCurrent, errMaxNorm);
    } else if(errMaxNorm >= 0) {
        return GrowStepSize(hstepCurrent, errMaxNorm);
    }

    // if error estimate is negative (dubious)
    return max_stepping_increase * hstepCurrent;
}

void G4MagInt_Driver::SetSmallestFraction(G4double newFraction)
{
    if( (newFraction > 1.e-16) && (newFraction < 1e-8) ) {
        fSmallestFraction = newFraction;
    } else {
        G4cerr << "Warning: SmallestFraction not changed. " << G4endl
               << "  Proposed value was " << newFraction << G4endl
               << "  Value must be between 1.e-8 and 1.e-16" << G4endl;
    }
}

void G4MagInt_Driver::GetDerivatives(
    const G4FieldTrack& track, // const, INput
    G4double dydx[]) const // OUTput
{
    G4double  y[G4FieldTrack::ncompSVEC];
    track.DumpToArray(y);
    pIntStepper -> RightHandSide(y, dydx);
}
