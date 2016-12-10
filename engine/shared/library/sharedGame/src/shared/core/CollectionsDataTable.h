// ======================================================================
//
// CollectionsDataTable.h
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_CollectionsDataTable_H
#define INCLUDED_CollectionsDataTable_H

#include "sharedFoundation/NetworkId.h"

#include <vector>

// ======================================================================

class CollectionsDataTable // static class
{
public:
	static void install();
	static void remove();

	enum ShowIfNotYetEarnedType
	{
		SE_gray,
		SE_unknown,
		SE_none
	};

	class CollectionInfoBook
	{
	public:
		CollectionInfoBook(std::string const & pName, std::string const & pIcon, ShowIfNotYetEarnedType pShowIfNotYetEarned, bool pHidden) :
		  name(pName), icon(pIcon), showIfNotYetEarned(pShowIfNotYetEarned), hidden(pHidden) {};

		  std::string const name;
		  std::string const icon;
		  ShowIfNotYetEarnedType const showIfNotYetEarned;
		  bool const hidden;

	private:
		CollectionInfoBook();
		CollectionInfoBook(CollectionInfoBook const &);
		CollectionInfoBook &operator =(CollectionInfoBook const &);
	};

	class CollectionInfoPage
	{
		public:
			CollectionInfoPage(std::string const & pName, std::string const & pIcon, ShowIfNotYetEarnedType pShowIfNotYetEarned, bool pHidden, std::vector<std::string> const & pTitles, CollectionInfoBook const & pBook) :
				name(pName), icon(pIcon), showIfNotYetEarned(pShowIfNotYetEarned), hidden(pHidden), titles(pTitles), book(pBook) {};

			std::string const name;
			std::string const icon;
			ShowIfNotYetEarnedType const showIfNotYetEarned;
			bool const hidden;
			std::vector<std::string> const titles;

			CollectionInfoBook const & book;

		private:
			CollectionInfoPage();
			CollectionInfoPage(CollectionInfoPage const &);
			CollectionInfoPage &operator =(CollectionInfoPage const &);
	};

	class CollectionInfoCollection
	{
		public:
			CollectionInfoCollection(std::string const & cName, std::string const & pIcon, ShowIfNotYetEarnedType pShowIfNotYetEarned, bool pHidden, std::vector<std::string> const & pCategories, std::vector<std::string> const & pTitles, bool pNoReward, bool pTrackServerFirst, CollectionInfoPage const & pPage) :
				name(cName), icon(pIcon), showIfNotYetEarned(pShowIfNotYetEarned), hidden(pHidden), categories(pCategories), titles(pTitles), noReward(pNoReward), trackServerFirst(pTrackServerFirst), serverFirstClaimTime(0), serverFirstClaimantId(), serverFirstClaimantName(), page(pPage) {};

			std::string const name;
			std::string const icon;
			ShowIfNotYetEarnedType const showIfNotYetEarned;
			bool const hidden;
			std::vector<std::string> const categories;
			std::vector<std::string> const titles;
			bool const noReward;
			bool const trackServerFirst;
			time_t const serverFirstClaimTime;
			NetworkId const serverFirstClaimantId;
			Unicode::String const serverFirstClaimantName;

			CollectionInfoPage const & page;

		private:
			CollectionInfoCollection();
			CollectionInfoCollection(CollectionInfoCollection const &);
			CollectionInfoCollection &operator =(CollectionInfoCollection const &);
	};

	class CollectionInfoSlot
	{
		friend class CollectionsDataTable;

		public:
			CollectionInfoSlot(std::string const & pName, std::string const & pIcon, ShowIfNotYetEarnedType pShowIfNotYetEarned, bool pHidden, bool pNotifyScriptOnModify, int pSlotIdIndex, int pBeginSlotId, int pAbsoluteBeginSlotId, int pEndSlotId, int pAbsoluteEndSlotId, unsigned long pMaxSlotValue, unsigned long pMaxValueForNumBits, std::vector<std::string> const & pCategories, std::vector<std::string> const & pPrereqs, std::string const & pMusic, std::vector<std::string> const & pTitles, CollectionInfoCollection const & pCollection) :
				name(pName), icon(pIcon), showIfNotYetEarned(pShowIfNotYetEarned), hidden(pHidden), notifyScriptOnModify(pNotifyScriptOnModify), slotIdIndex(pSlotIdIndex), beginSlotId(pBeginSlotId), absoluteBeginSlotId(pAbsoluteBeginSlotId), endSlotId(pEndSlotId), absoluteEndSlotId(pAbsoluteEndSlotId), counterTypeSlot(pEndSlotId > 0), maxSlotValue(pMaxSlotValue), maxValueForNumBits(pMaxValueForNumBits), categories(pCategories), prereqsPtr(), music(pMusic), titles(pTitles), collection(pCollection), prereqs(pPrereqs) {};

			std::string const name;
			std::string const icon;
			ShowIfNotYetEarnedType const showIfNotYetEarned;
			bool const hidden;
			bool const notifyScriptOnModify;

			int const slotIdIndex;
			int const beginSlotId;
			int const absoluteBeginSlotId;
			int const endSlotId;
			int const absoluteEndSlotId;
			bool const counterTypeSlot;
			unsigned long const maxSlotValue;

			// for bit-type slot, 1
			// for counter-type slot, the maximum value that can be stored in the number of bits (endSlotId - beginSlotId + 1)
			unsigned long const maxValueForNumBits;

			std::vector<std::string> const categories;
			std::vector<CollectionInfoSlot const *> const prereqsPtr;
			std::string const music;
			std::vector<std::string> const titles;
			CollectionInfoCollection const & collection;

		private:
			std::vector<std::string> const prereqs;

			CollectionInfoSlot();
			CollectionInfoSlot(CollectionInfoSlot const &);
			CollectionInfoSlot &operator =(CollectionInfoSlot const &);
	};

	// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
	// owned by the caller, they are owned/managed/freed by this class

	// slots
	static CollectionsDataTable::CollectionInfoSlot const * getSlotByName(std::string const & slotName);
	static CollectionsDataTable::CollectionInfoSlot const * getSlotByBeginSlotId(int slotId);
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & getAllTitleableSlots();
	static CollectionsDataTable::CollectionInfoSlot const * isASlotTitle(std::string const & titleName);

	// collections
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & getSlotsInCollection(std::string const & collectionName);
	static std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & getAllTitleableCollections();
	static CollectionsDataTable::CollectionInfoCollection const * isACollectionTitle(std::string const & titleName);
	static std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *> const & getAllServerFirstCollections();
	static CollectionsDataTable::CollectionInfoCollection const * getCollectionByName(std::string const & collectionName);

	// pages
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & getSlotsInPage(std::string const & pageName);
	static std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & getCollectionsInPage(std::string const & pageName);
	static std::vector<CollectionsDataTable::CollectionInfoPage const *> const & getAllTitleablePages();
	static CollectionsDataTable::CollectionInfoPage const * isAPageTitle(std::string const & titleName);
	static CollectionsDataTable::CollectionInfoPage const * getPageByName(std::string const & pageName);

	// books
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & getSlotsInBook(std::string const & bookName);
	static std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & getCollectionsInBook(std::string const & bookName);
	static std::vector<CollectionsDataTable::CollectionInfoPage const *> const & getPagesInBook(std::string const & bookName);
	static std::vector<CollectionsDataTable::CollectionInfoBook const *> const & getAllBooks();
	static CollectionsDataTable::CollectionInfoBook const * getBookByName(std::string const & bookName);

	// categories
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & getSlotsInCategory(std::string const & categoryName);
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & getSlotsInCategoryByCollection(std::string const & collectionName, std::string const & categoryName);
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & getSlotsInCategoryByPage(std::string const & pageName, std::string const & categoryName);
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & getSlotsInCategoryByBook(std::string const & bookName, std::string const & categoryName);

	static std::set<std::string> const & getAllSlotCategoriesInCollection(std::string const & collectionName);
	static std::set<std::string> const & getAllSlotCategoriesInPage(std::string const & pageName);
	static std::set<std::string> const & getAllSlotCategoriesInBook(std::string const & bookName);
	static std::set<std::string> const & getAllSlotCategories();

	// misc
	static Unicode::String localizeCollectionName(std::string const & name);
	static Unicode::String localizeCollectionDescription(std::string const & name);
	static Unicode::String localizeCollectionTitle(std::string const & name);
	static char const * getShowIfNotYetEarnedTypeString(ShowIfNotYetEarnedType const showIfNotYetEarned);

	// "server first" processing
	static void setServerFirstData(std::set<std::pair<std::pair<int32, std::string>, std::pair<NetworkId, Unicode::String> > > const & collectionServerFirst);

private: // disabled

	CollectionsDataTable();
	CollectionsDataTable(CollectionsDataTable const &);
	CollectionsDataTable &operator =(CollectionsDataTable const &);
};

// ======================================================================

#endif // INCLUDED_CollectionsDataTable_H
