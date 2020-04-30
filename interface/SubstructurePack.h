#ifndef DataFormats_SubstructurePack_h
#define DataFormats_SubstructurePack_h

#include <vector>
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/JetReco/interface/BasicJet.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
 #include <TLorentzVector.h>

class SubstructurePack {
    public:
        typedef edm::Ptr<reco::GenJet> GenJetPtr;
        typedef edm::Ptr<reco::BasicJet> BasicJetPtr;
        typedef edm::Ptr<reco::GenParticle> GenParticlePtr;

        // Constructor & destructor (default needed)
        inline SubstructurePack() {}
        inline SubstructurePack(const GenJetPtr fGenJet) {genJet_ = *fGenJet;}
        virtual ~SubstructurePack() {};

        // Getters
        const reco::GenJet *      jet()             const { return &genJet_; }
        const reco::BasicJet *    substructurejet() const { return &substructurejet_; }
        const reco::GenParticle * zprime()          const { return &zprime_ ;}
        std::vector<GenJetPtr>    subjets()         const { return subjets_; }
        int                       nSubjets()        const { return subjets_.size(); }
        bool                      hasZprime()       const { return hasZprime_ ;}
        TLorentzVector            summedsubjets()   const { return summedsubjets_ ;}

        // Setters

        void addZprime( const GenParticlePtr fZprime ) {
            zprime_ = *fZprime ;
            hasZprime_ = true ;
            }

        void addSubstructure(const BasicJetPtr fSubstructureJet){
            substructurejet_ = *fSubstructureJet;
            // The daughters of substructurejet_ will be the actual subjets
            for ( auto const & subjet : substructurejet_.daughterPtrVector()) {
                addSubjet( edm::Ptr<reco::GenJet>(subjet) ) ;
                }
            // Also calculate the summed subjets
            summedsubjets_ = TLorentzVector(0., 0., 0., 0.);
            for ( auto const & subjet : subjets_) {
                summedsubjets_ += TLorentzVector(subjet->px(), subjet->py(), subjet->pz(), subjet->energy());
                }
            }

        void addSubjet(GenJetPtr fSubjet) {
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
            subjets_.push_back(fSubjet) ;
            }

        std::vector<reco::CandidatePtr> allSubjetDaughters() {
            /* Puts all daughters of the class' subjets in one vector and returns */
            std::vector<reco::CandidatePtr> _allSubjetDaughters ;
            for ( const auto & subjet : subjets_) {
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
        reco::GenJet genJet_;
        reco::GenParticle zprime_;
        reco::BasicJet substructurejet_;
        bool hasZprime_ = false ;
        std::vector<edm::Ptr<reco::GenJet>> subjets_;
        TLorentzVector summedsubjets_;
    };

typedef std::vector<SubstructurePack> SubstructurePackCollection;

#endif