#ifndef SSJ_ALLPAIRS_H
#define SSJ_ALLPAIRS_H

/* Copyright 2014-2015 Willi Mann
 *
 * This file is part of set_sim_join.
 *
 * set_sim_join is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with set_sim_join.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <vector>
#include "classes.h"
#include "output.h"
#include "verify.h"
#include "data.h"
#include "lengthfilter.h"
#include "frequencysorting.h"
#include "indexes.h"
#include "allpairs_policies.h"
#include "candidateset.h"

template <typename AllPairsSimilarity/* = Jaccard*/,
		 typename AllPairsIndexingStrategyPolicy = IndexOnTheFlyPolicy,
		 typename AllPairsLengthFilterPolicy = DefaultLengthFilterPolicy
		 >
class AllPairs: public Algorithm {
	public:
		typedef typename AllPairsSimilarity::threshold_type threshold_type;
		const threshold_type threshold;

		/* Terminology:
		   ForeignRecords .. Only for foreign joins - contain sets to probe against index
		   IndexedRecords .. Records that will be indexed - in case of self-joins, identical to probing set
		   */
		typedef IntRecord BaseRecord;
		typedef IntRecord ForeignRecord;
		typedef IntRecords ForeignRecords;

		struct CandidateData {
			unsigned int count;
			void reset() {
				count = 0;
			}
			CandidateData() : count(0) {}
		};

		class IndexedRecord : public ForeignRecord {
			public:
				typename AllPairsIndexPolicy::IndexStructureRecordData structuredata;
				IndexedRecord() {}
				inline void cleanup() {
				}
				CandidateData candidateData;
		};
		typedef std::vector<IndexedRecord> IndexedRecords;
	private:
		ForeignRecords foreignrecords;
		IndexedRecords indexedrecords;
		algo_handle_records_freq_sort<IndexedRecords, ForeignRecords> inputhandler;


	public:
		typedef AllPairs<AllPairsSimilarity, AllPairsIndexingStrategyPolicy, AllPairsLengthFilterPolicy> self_type;

		typedef std::vector<IntRecord> Records;
		typedef IntRecord Record;

		typedef CandidateSet<CandidateData, IndexedRecords> CandidateSet_;
		typedef AllPairsSimilarity Similarity;
		typedef AllPairsIndexPolicy IndexStructurePolicy;
		typedef typename IndexStructurePolicy::template IndexStructure<self_type> IndexStructure;
		typedef AllPairsLengthFilterPolicy LengthFilterPolicy;
		typedef AllPairsIndexingStrategyPolicy IndexingStrategyPolicy;
		typedef typename AllPairsIndexingStrategyPolicy::template Index<self_type> Index;

		Index index;

	public:
		//constructor
		AllPairs(threshold_type threshold) : threshold(threshold), inputhandler(indexedrecords, foreignrecords) {}

		//addrecord and addforeignrecord must use swap to get the integer vector from record,
		//such that record refers to an empty record after the respective calls
		void addrecord(IntRecord & record);
		void addforeignrecord(IntRecord & record);

		//addrawrecord and addrawforeignrecord must use swap to get the integer vector from record,
		//such that record refers to an empty record after the respective calls
		void addrawrecord(IntRecord & record);
		void addrawforeignrecord(IntRecord & record);

		//multi-step process to measure the individual steps from outside
		void preparerecords();
		void prepareforeignrecords();
		void preparefinished();
		void doindex();
		void dojoin(
				HandleOutput * handleoutput);

		virtual ~AllPairs();

	private:
		inline size_t proberecordssize() {
			if(Index::SELF_JOIN) {
				return indexedrecords.size();
			} else {
				return foreignrecords.size();
			}
		}
		GetProbeRecord<Index::SELF_JOIN,  ForeignRecords, IndexedRecords,
			IndexedRecord, typename Index::ProbeRecord> getproberecord;
};


template <typename AllPairsSimilarity, class AllPairsIndexingStrategyPolicy, class AllPairsLengthFilterPolicy>
void AllPairs<AllPairsSimilarity, AllPairsIndexingStrategyPolicy, AllPairsLengthFilterPolicy>::addrecord(IntRecord & record) {
	inputhandler.addrecord(record);
}

template <typename AllPairsSimilarity, class AllPairsIndexingStrategyPolicy, class AllPairsLengthFilterPolicy>
void AllPairs<AllPairsSimilarity, AllPairsIndexingStrategyPolicy, AllPairsLengthFilterPolicy>::addforeignrecord(IntRecord & record) {
	inputhandler.addforeignrecord(record);
}

template <typename AllPairsSimilarity, class AllPairsIndexingStrategyPolicy, class AllPairsLengthFilterPolicy>
void AllPairs<AllPairsSimilarity, AllPairsIndexingStrategyPolicy, AllPairsLengthFilterPolicy>::addrawrecord(IntRecord & record) {
	inputhandler.addrawrecord(record);
}

template <typename AllPairsSimilarity, class AllPairsIndexingStrategyPolicy, class AllPairsLengthFilterPolicy>
void AllPairs<AllPairsSimilarity, AllPairsIndexingStrategyPolicy, AllPairsLengthFilterPolicy>::addrawforeignrecord(IntRecord & record) {
	inputhandler.addrawforeignrecord(record);
}

template <typename AllPairsSimilarity, class AllPairsIndexingStrategyPolicy, class AllPairsLengthFilterPolicy>
void AllPairs<AllPairsSimilarity, AllPairsIndexingStrategyPolicy, AllPairsLengthFilterPolicy>::preparerecords() {
	inputhandler.prepareindex();

}
template <typename AllPairsSimilarity, class AllPairsIndexingStrategyPolicy, class AllPairsLengthFilterPolicy>
void AllPairs<AllPairsSimilarity, AllPairsIndexingStrategyPolicy, AllPairsLengthFilterPolicy>::prepareforeignrecords() {
	inputhandler.prepareforeign();

}

template <typename AllPairsSimilarity, class AllPairsIndexingStrategyPolicy, class AllPairsLengthFilterPolicy>
void AllPairs<AllPairsSimilarity, AllPairsIndexingStrategyPolicy, AllPairsLengthFilterPolicy>::preparefinished() {
	inputhandler.cleanup();

}

template <typename AllPairsSimilarity, class AllPairsIndexingStrategyPolicy, class AllPairsLengthFilterPolicy>
void AllPairs<AllPairsSimilarity,AllPairsIndexingStrategyPolicy, AllPairsLengthFilterPolicy>::doindex() {
	index.largest_tokenid(inputhandler.get_largest_tokenid());
	index.index_records(indexedrecords, threshold);

}

template <typename AllPairsSimilarity, class AllPairsIndexingStrategyPolicy, class AllPairsLengthFilterPolicy>
void AllPairs<AllPairsSimilarity, AllPairsIndexingStrategyPolicy, AllPairsLengthFilterPolicy>::dojoin(
				HandleOutput * handleoutput) {

	CandidateSet_ candidateSet(indexedrecords);

	std::vector<unsigned int> minoverlapcache;
	unsigned int lastprobesize = 0;

	// foreach record...
	for (unsigned recind = 0; recind < proberecordssize(); ++recind) {
		typename Index::ProbeRecord & record = getproberecord(indexedrecords, foreignrecords, recind);
		unsigned int reclen = record.tokens.size();
		unsigned int maxLen = indexedrecords[indexedrecords.size() - 1].tokens.size();

		//std::cout << reclen << "   " << indexedrecords[indexedrecords.size() - 1].tokens.size() << std::endl;

		//Minimum size of records in index
		unsigned int minsize = Similarity::minsize(reclen, threshold);

		// Check whether cache is to renew
		if(lastprobesize != reclen) {
			lastprobesize = reclen;
			unsigned int maxel = Index::SELF_JOIN ? reclen : Similarity::maxsize(reclen, threshold,maxLen);
			minoverlapcache.resize(maxel + 1);
			for(unsigned int i = minsize; i <= maxel; ++i) {
				minoverlapcache[i] = Similarity::minoverlap(reclen, i, threshold);
			}
		}

		// Length of probing prefix
		unsigned int maxprefix = Similarity::maxprefix(reclen, threshold);

		typename AllPairsIndexingStrategyPolicy::template maxsizechecker<self_type> 
			maxsizechecker(reclen, threshold, maxLen);


		// foreach elem in probing prefix
		for (unsigned recpos = 0; recpos < maxprefix; ++recpos) {
			unsigned int token = record.tokens[recpos];

			// get iterator and do min length filtering at the start
			typename Index::iterator ilit = index.getiterator(token);

			statistics.lookups.inc();

			maxsizechecker.updateprobepos(recpos);

			// First, apply min-length filter
			while(!ilit.end()) {
				// record from index
				IndexedRecord & indexrecord = indexedrecords[ilit->recordid];
				unsigned int indreclen = indexrecord.tokens.size();

				//Length filter - check whether minsize is satisfied
				if(ilit.lengthfilter(indreclen, minsize)) {
					break;
				}
				//Note: the iterator is increased by lengthfilter
			}

			// for each record in inverted list 
			while(!ilit.end() ) {

				if(!AllPairsIndexingStrategyPolicy::recindchecker::istocheck(recind, ilit->recordid)) {
					break;
				}

				// record from index 
				IndexedRecord & indexrecord = indexedrecords[ilit->recordid];
				unsigned int indreclen = indexrecord.tokens.size();

				//Length filter 2 - maxlength above tighter length filter (if enabled)
				if(maxsizechecker.isabove(indreclen)) {
					break;
				}

				statistics.indexEntriesSeen.inc();

				// insert candidate if it was not already seen
				CandidateData & candidateData = candidateSet.getCandidateData(ilit->recordid);

				if(candidateData.count == 0) {
					candidateSet.addRecord(ilit->recordid);
				}
				candidateData.count += 1;

				ilit.next();
			}
		}
		statistics.candidatesP1.add(candidateSet.size());

		//Now, verify candidates
		typename CandidateSet_::iterator candit = candidateSet.begin();
		for( ; candit != candidateSet.end(); ++candit) {
			CandidateData & candidateData = candidateSet.getCandidateData(*candit);

			statistics.candidatesVery.inc();
#ifndef CAND_ONLY
			// record from index
			const IndexedRecord & indexrecord = indexedrecords[*candit];
			unsigned int indreclen = indexrecord.tokens.size();
			
			unsigned int minoverlap = minoverlapcache[indreclen];

			//First position after last position by index lookup in indexed record
			unsigned int lastposind = IndexStructure::verify_indexrecord_start(indexrecord, indreclen, this);
			
			//First position after last position by index lookup in probing record
			unsigned int lastposprobe = LengthFilterPolicy::verify_record_start(reclen, maxprefix, minoverlap);

			unsigned int recpreftoklast = record.tokens[lastposprobe - 1];
			unsigned int indrecpreftoklast = indexrecord.tokens[lastposind - 1];

			unsigned int recpos, indrecpos;

			if(recpreftoklast > indrecpreftoklast) {

				recpos = candidateData.count;
				//first position after minprefix / lastposind
				indrecpos = lastposind;
			} else {
				// First position after maxprefix / lastposprobe
				recpos = lastposprobe;
				indrecpos = candidateData.count;
			}

			if(verifypair(record.tokens, indexrecord.tokens, minoverlap, recpos, indrecpos, candidateData.count)) {
				handleoutput->addPair(record, indexrecord);
			}
#endif
			candidateData.reset();
		}
		candidateSet.clear();

		index.index_record(record, recind, reclen, threshold);

	}
}

template <typename MpJoinSimilarity, class MpJoinIndexingStrategyPolicy, class MpJoinLengthFilterPolicy>
AllPairs<MpJoinSimilarity, MpJoinIndexingStrategyPolicy, MpJoinLengthFilterPolicy>::~AllPairs() {
}


#endif
