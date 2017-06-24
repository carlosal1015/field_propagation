#include "Utils.hh"

namespace magneticfield {

G4double relativeError(
    const G4double y[],
    const G4double yError[],
    const G4double h,
    const G4double errorTolerance)
{
    // Accuracy for position
    G4double error2 = extractValue2(yError, Value3D::Position) / sqr(h);

    // Accuracy for momentum
    const G4double momentum2 = extractValue2(y, Value3D::Momentum);
    if (momentum2 > 0) {
       const G4double momentumError2 =
           extractValue2(yError,  Value3D::Momentum) / momentum2;
       error2 = std::max(error2, momentumError2);
    } else {
        G4Exception("G4MagInt_Driver","Field001",
                    JustWarning, "found case of zero momentum");
    }
#if 0
    // Accuracy for spin
    const G4double spin2 = extractValue2(y, Value3D::Spin);
    if (spin2 > 0) {
        const G4double spinError2 = extractValue2(yError, Value3D::Spin) / spin2;
        error2 = std::max(error2, spinError2);
    }
#endif
    return std::sqrt(error2) / errorTolerance;
}

} // nagneticfield
