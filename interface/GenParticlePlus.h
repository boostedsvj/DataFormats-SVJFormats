#ifndef DataFormats_GenParticlePlus_h
#define DataFormats_GenParticlePlus_h

#include <vector>
#include <set>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

class GenParticlePlus : public reco::GenParticle {
    public:
        inline GenParticlePlus() {}
        inline GenParticlePlus(const reco::GenParticle & particle) : reco::GenParticle(particle) {}

        reco::GenParticle toGenParticle(const reco::Candidate * candidate) {
            reco::GenParticle particle(
                candidate->charge(), candidate->p4(), candidate->vertex(),
                candidate->pdgId(), candidate->status(), candidate->charge()
                );
            return particle;
            }

        std::vector<const reco::Candidate *> traverseDaughters() {
            return traverseDaughters(this);
            }

        std::vector<const reco::Candidate *> traverseDaughters(const reco::Candidate * node) {
            // Creates empty vector `returnable` which is subsequently returned
            std::vector<const reco::Candidate *> returnable;
            traverseDaughtersInPlace(node, returnable);
            return returnable;
            }

        void traverseDaughtersInPlace(
            // Only modifies the `returnable` vector
            const reco::Candidate * node,
            std::vector<const reco::Candidate *> & returnable
            ){
            returnable.push_back(node);
            if (node->numberOfDaughters() > 0){
                for(unsigned int i=0; i < node->numberOfDaughters(); i++){
                    traverseDaughtersInPlace(node->daughter(i), returnable);
                    }
                }
            }

        void setQuarksAndFinalProducts() {
            std::vector<const reco::Candidate *> daughters = traverseDaughters();

            std::set<const reco::Candidate *> allFinalProductsDark;
            std::set<const reco::Candidate *> allFinalProductsVisible;
            std::set<const reco::Candidate *> darkQuarkSet;
            for(unsigned int i=0; i < daughters.size(); i++){
                int pdgid = abs(daughters[i]->pdgId());
                if (pdgid == 51 || pdgid == 52 || pdgid == 53){
                    // The mothers of particles with pdgid 51-53 are the final dark particles
                    allFinalProductsDark.insert(daughters[i]->mother());
                    }
                else if (pdgid == 4900101 && daughters[i]->mother()->pdgId() == 4900023){
                    // Dark quarks (4900101) that have as mother the Z' are the initial dark quarks 
                    darkQuarkSet.insert(daughters[i]);
                    }
                else if (pdgid < 4000000 && daughters[i]->status() == 1) {
                    allFinalProductsVisible.insert(daughters[i]);
                    }
                }

            if (darkQuarkSet.size() != 2){
                throw cms::Exception("NotTwoDarkQuarks")
                    << "Found " << darkQuarkSet.size() << " dark quarks, but expected exactly 2\n";
                }

            // Convert to vector
            std::vector<const reco::Candidate *> darkQuarkCandidates(darkQuarkSet.begin(), darkQuarkSet.end());

            // Add to class variable
            for(auto const c : darkQuarkCandidates) {
                reco::GenParticle darkQuark = toGenParticle(c);
                darkQuarks.push_back(darkQuark);
                }

            for(auto const c : allFinalProductsDark) {
                const reco::Candidate * mother = c;
                reco::GenParticle particle = toGenParticle(c);
                while (mother->numberOfMothers() > 0){
                    if (mother == darkQuarkCandidates[0]){
                        finalDarkParticlesLeft.push_back(particle);
                        break;
                        }
                    else if (mother == darkQuarkCandidates[1]){
                        finalDarkParticlesRight.push_back(particle);
                        break;
                        }
                    mother = mother->mother();
                    }
                }

            for(auto const c : allFinalProductsVisible) {
                const reco::Candidate * mother = c;
                reco::GenParticle particle = toGenParticle(c);
                while (mother->numberOfMothers() > 0){
                    if (mother == darkQuarkCandidates[0]){
                        finalVisibleParticlesLeft.push_back(particle);
                        break;
                        }
                    else if (mother == darkQuarkCandidates[1]){
                        finalVisibleParticlesRight.push_back(particle);
                        break;
                        }
                    mother = mother->mother();
                    }
                }
            }

        std::vector<reco::GenParticle> darkQuarks;
        std::vector<reco::GenParticle> finalDarkParticlesLeft;
        std::vector<reco::GenParticle> finalDarkParticlesRight;
        std::vector<reco::GenParticle> finalVisibleParticlesLeft;
        std::vector<reco::GenParticle> finalVisibleParticlesRight;
    };

typedef std::vector<GenParticlePlus> GenParticlePlusCollection;
#endif