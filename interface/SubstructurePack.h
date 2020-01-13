#ifndef DataFormats_SubstructurePack_h
#define DataFormats_SubstructurePack_h

#include <vector>
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"

class SubstructurePack {
    public:
        // Constructor & destructor (default needed)
        inline SubstructurePack() {}
        inline SubstructurePack(const reco::GenJet * fGenJet) {
            _genJet = fGenJet;
            }
        virtual ~SubstructurePack() {};

        const reco::GenJet * jet() const { return _genJet; }
        std::vector<edm::Ptr<reco::GenJet>> subjets() const { return _subjets; }
        bool hasZprime() const { return _hasZprime ;}
        const reco::GenParticle * zprime() const { return _zprime ;}

        void addZprime( const reco::GenParticle * fZprime ) {
            _zprime = fZprime ;
            _hasZprime = true ;
            }

        void addSubjet( edm::Ptr<reco::GenJet> fSubjet ) {
            // edm::Ptr<reco::GenJet>(fSubjet)
            // check that the subjet does not contain any extra constituents not contained in the jet
            // If subjetDaughter is not a daughter of the main jet, skip the whole subjet
            const std::vector<reco::CandidatePtr> & jetDaughters = jet()->daughterPtrVector();
            for (const reco::CandidatePtr & subjetDaughter : fSubjet->daughterPtrVector()) {
                if (std::find(jetDaughters.begin(), jetDaughters.end(), subjetDaughter) == jetDaughters.end()) {
                    // There is a constituent of the subjet that is not a daughter of the jet
                    // Skip the whole subjet
                    return ;
                    }
                }
            _subjets.push_back(fSubjet) ;
            }

        std::vector<reco::CandidatePtr> allSubjetDaughters() {
            /* Puts all daughters of the class' subjets in one vector and returns */
            std::vector<reco::CandidatePtr> _allSubjetDaughters ;
            for ( const auto & subjet : _subjets) {
                const std::vector<reco::CandidatePtr> & subjetDaughters = subjet->daughterPtrVector();
                _allSubjetDaughters.insert(_allSubjetDaughters.end(), subjetDaughters.begin(), subjetDaughters.end());
                }
            return _allSubjetDaughters ;
            }

        std::vector<reco::CandidatePtr> daughtersNotInSubjets() {
            /* Returns all the jet daughters that were not accounted for in the subjets */
            std::vector<reco::CandidatePtr> _leftOverDaughters ;
            std::vector<reco::CandidatePtr> _allSubjetDaughters = allSubjetDaughters();
            for (const reco::CandidatePtr & jetDaughter : jet()->daughterPtrVector()) {
                if (std::find(_allSubjetDaughters.begin(), _allSubjetDaughters.end(), jetDaughter) == _allSubjetDaughters.end()) {
                    // jetDaughter is not in any subjet
                    _leftOverDaughters.push_back(jetDaughter);
                    }
                }
            return _leftOverDaughters ;
            }


    private:
        const reco::GenJet * _genJet;
        const reco::GenParticle * _zprime;
        bool _hasZprime = false ;
        std::vector<edm::Ptr<reco::GenJet>> _subjets;
    };

typedef std::vector<SubstructurePack> SubstructurePackCollection;

#endif