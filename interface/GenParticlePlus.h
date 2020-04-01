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

        void printDecayTree() {
            traverseDaughters(this, true);
            }

        std::vector<const reco::Candidate *> traverseDaughters() {
            return traverseDaughters(this);
            }

        std::vector<const reco::Candidate *> traverseDaughters(const reco::Candidate * node, bool verbose = false) {
            // Creates empty vector `returnable` which is subsequently returned
            std::vector<const reco::Candidate *> returnable;
            traverseDaughtersInPlace(node, returnable, verbose);
            return returnable;
            }

        void traverseDaughtersInPlace(
            // Only modifies the `returnable` vector
            const reco::Candidate * node,
            std::vector<const reco::Candidate *> & returnable,
            bool verbose = false,
            unsigned int depth = 0
            ){
            // Work on current node
            if (verbose) {
                // Only print, don't save node
                std::string toprint;
                // Add indentation
                for(unsigned int i=0; i < depth; i++){toprint += "....";}
                toprint += std::to_string(node->pdgId());
                // edm::LogError("TreeTrav") << toprint;
                std::cout << toprint << std::endl;
                }
            else {
                // Save node in output vectors
                returnable.push_back(node);
                }
            // Recursion
            if (node->numberOfDaughters() > 0){
                for(unsigned int i=0; i < node->numberOfDaughters(); i++){
                    traverseDaughtersInPlace(node->daughter(i), returnable, verbose, depth+1);
                    }
                }
            }

        void copyCandidateSetToGenParticleVector(
            std::set<const reco::Candidate *> & candidateSet,
            std::vector<reco::GenParticle> & particleVector
            ){
            for(auto const c : candidateSet) {
                reco::GenParticle particle = toGenParticle(c);
                particleVector.push_back(particle);
                }
            }

        bool has4900101daughters(const reco::Candidate * darkQuark){
            if (darkQuark->numberOfDaughters() == 0){ return false; }
            for(unsigned int i=0; i < darkQuark->numberOfDaughters(); i++){
                if (abs(darkQuark->daughter(i)->pdgId()) == 4900101){
                    return true;
                    }
                }
            return false;
            }

        bool isDarkMeson(const reco::Candidate * candidate){
            int pdgid = abs(candidate->pdgId());
            return bool( pdgid >= 4900111 && pdgid < 4901000 );
            }

        bool decaysDarkly(const reco::Candidate * candidate){
            int pdgid = abs(candidate->daughter(0)->pdgId());
            return bool( pdgid == 51 || pdgid == 52 || pdgid == 53 );
            }

        void setQuarksAndFinalProducts() {
            std::vector<const reco::Candidate *> daughters = traverseDaughters();
            // Categorize daughters of the Z' into sets
            std::set<const reco::Candidate *> darkMesonDecayingDarkSet;
            std::set<const reco::Candidate *> darkMesonDecayingVisibleSet;
            std::set<const reco::Candidate *> initialDarkQuarkSet;
            std::set<const reco::Candidate *> finalDarkQuarkSet;
            std::set<const reco::Candidate *> finalVisibleProductSet;
            for(unsigned int i=0; i < daughters.size(); i++){
                int pdgid = abs(daughters[i]->pdgId());
                const reco::Candidate * daughter = daughters[i];
                // Deal with dark mesons
                if (isDarkMeson(daughter)){
                    if (decaysDarkly(daughter)){
                        darkMesonDecayingDarkSet.insert(daughter);
                        }
                    else {
                        darkMesonDecayingVisibleSet.insert(daughter);
                        }
                    }
                // Deal with dark quarks
                else if (pdgid == 4900101){
                    if (daughter->mother()->pdgId() == 4900023){
                        // Dark quarks (4900101) that have as mother the Z' are the initial dark quarks 
                        initialDarkQuarkSet.insert(daughter);
                        }
                    else if (abs(daughter->mother()->pdgId()) == 4900101 && !has4900101daughters(daughter)){
                        // Dark quarks (4900101) that have no dark quark daughters are the final dark quarks 
                        finalDarkQuarkSet.insert(daughter);
                        }
                    }
                // Deal with final visible particles
                else if (
                    (pdgid < 4000000 || pdgid > 5000000)
                    && !( pdgid==51 || pdgid==52 || pdgid==53 )
                    && daughter->status() == 1)
                    {
                    finalVisibleProductSet.insert(daughter);
                    }
                }
            // Assert there are exactly two quarks initially
            if (initialDarkQuarkSet.size() != 2) {
                throw cms::Exception("NotTwoDarkQuarks")
                    << "Found " << initialDarkQuarkSet.size() << " dark quarks, but expected exactly 2\n";
                }
            // Copy sets into the class vectors as GenParticles
            copyCandidateSetToGenParticleVector(darkMesonDecayingDarkSet, darkMesonDecayingDark);
            copyCandidateSetToGenParticleVector(darkMesonDecayingVisibleSet, darkMesonDecayingVisible);
            copyCandidateSetToGenParticleVector(initialDarkQuarkSet, initialDarkQuark);
            copyCandidateSetToGenParticleVector(finalDarkQuarkSet, finalDarkQuark);
            copyCandidateSetToGenParticleVector(finalVisibleProductSet, finalVisibleProduct);
            }

        std::vector<reco::GenParticle> darkMesonDecayingDark;
        std::vector<reco::GenParticle> darkMesonDecayingVisible;
        std::vector<reco::GenParticle> initialDarkQuark;
        std::vector<reco::GenParticle> finalDarkQuark;
        std::vector<reco::GenParticle> finalVisibleProduct;
    };

typedef std::vector<GenParticlePlus> GenParticlePlusCollection;
#endif