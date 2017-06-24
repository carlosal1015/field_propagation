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
// $Id: NTSTDetectorMessenger.hh 66241 2012-12-13 18:34:42Z gunter $
//
// 

#ifndef NTST_DETECTOR_MESSENGER_HH
#define NTST_DETECTOR_MESSENGER_HH

#include "globals.hh"
#include "G4UImessenger.hh"

#include <map>

class NTSTDetectorConstruction;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWithoutParameter;

class NTSTDetectorMessenger: public G4UImessenger {
public:
    enum class StepperType {
        ClassicalRK4,
        CashKarp,
        DormandPrince
    };

    enum class DriverType {
        G4MagInt_Driver,
        G4IntegrationDriver,
        G4FSALIntegrationDriver
    };

    NTSTDetectorMessenger(NTSTDetectorConstruction* );
   ~NTSTDetectorMessenger();
    
    void SetNewValue(G4UIcommand*, G4String);
    
private:
    NTSTDetectorConstruction* NTSTDetector;
    
    G4UIdirectory* NTSTdetDir;
    G4UIcmdWithAnInteger* DebugCmd;
    G4UIcmdWithAnInteger* NSubLayer;
    G4UIcmdWithADoubleAndUnit* MotherOuterRadius;
    G4UIcmdWithAString* InputFileNameCmd;
    G4UIcmdWithAString* DisableDet;
    G4UIcmdWithoutParameter* fieldStat;

    G4UIcmdWithAString* StepperMethodCmd;
    G4UIcmdWithAString* DriverMethodCmd;
};

#endif
