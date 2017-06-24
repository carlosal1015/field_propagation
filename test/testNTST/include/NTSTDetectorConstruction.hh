//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// This code implementation is the intellectual property of
// the RD44 GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms.
//
// $Id: NTSTDetectorConstruction.hh 66241 2012-12-13 18:34:42Z gunter $
//

#ifndef NTSTDetectorConstruction_HH
#define NTSTDetectorConstruction_HH

#include "G4Transform3D.hh"
#include "globals.hh"
#include "G4ChordFinder.hh"
#include "G4VFSALIntegrationStepper.hh"
#include "NTSTDetectorMessenger.hh"

class NTSTFileRead;
class G4VPhysicalVolume;
class G4LogicalVolume;

#include "G4VUserDetectorConstruction.hh"

#include "NTSTField.hh"


class NTSTDetectorConstruction : public G4VUserDetectorConstruction {
public:
    NTSTDetectorConstruction();
    ~NTSTDetectorConstruction();
    void SetInputFileName(G4String);
    void SetDebugCmd(G4int);
    void SetOuterRadius(G4double);
    void SetNSubLayer(G4int);
    void PrintCorners(const G4Transform3D&, G4LogicalVolume*);
    void DisableDetector(G4String);
    
    G4VPhysicalVolume* Construct();
    
    void GetFieldCallStats()
    {
    	G4cout << "Number calls to field = " << field.GetCount() << G4endl;
        field.ClearCount();
    }

    void SetStepperMethod(NTSTDetectorMessenger::StepperType stepperType);
    void SetDriverMethod(NTSTDetectorMessenger::DriverType driverType);

private:
    void constructField();

    NTSTFileRead* _FileRead;
    G4bool debug;
    G4double radius; // outer radius of the SVT mother volume
    NTSTDetectorMessenger* DetectorMessenger;
    G4int NSubLayer; // default number of layers
    G4bool disableSVT;
    G4bool disableDCH;
    
    NTSTField field;
    G4double  fMinChordStep;   // Minimum Step for chord

    G4EquationOfMotion* fEquation;
    G4MagIntegratorStepper* fStepper;
    G4VFSALIntegrationStepper* fFSALStepper;
    G4VIntegrationDriver* fDriver;
    G4ChordFinder* fChordFinder;
};

#endif
