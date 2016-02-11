// ======================================================================
//
// CollectionsDataTable.cpp
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CollectionsDataTable.h"

#include "UnicodeUtils.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/DataTable.h"

#include <map>
#include <set>
#include <vector>

// ======================================================================

namespace CollectionsDataTableNamespace
{
	char const * const cs_collectionsDataTableName = "datatables/collection/collection.iff";

	//----------------------------------------------------------------------
	// slots

	// all slots by slot name
	std::map<std::string, CollectionsDataTable::CollectionInfoSlot const *> s_allSlotsByName;

	// all slots by begin slot id
	// for quick lookup of a slot by begin slot id, we use a vector because
	// slot id are guaranteed to start at 0 and be contiguous; for counter-type
	// slot, only the begin slot id index points to the slot; the other slot
	// id indices point to nullptr
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> s_allSlotsById;

	// all title(able) slots
	std::vector<CollectionsDataTable::CollectionInfoSlot const *> s_allTitleableSlots;

	// all slot titles
	std::map<std::string, CollectionsDataTable::CollectionInfoSlot const *> s_allSlotTitles;

	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	// collections

	// slots in each collection
	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoSlot const *> > s_slotsInCollection;

	// all title(able) collections
	std::vector<CollectionsDataTable::CollectionInfoCollection const *> s_allTitleableCollections;

	// all collection titles
	std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *> s_allCollectionTitles;

	// all collections that have "server first" tracking
	std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *> s_allServerFirstCollections;

	// all collections by collection name
	std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *> s_allCollectionsByName;

	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	// pages

	// slots in each page
	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoSlot const *> > s_slotsInPage;

	// collections in each page
	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoCollection const *> > s_collectionsInPage;

	// all title(able) pages
	std::vector<CollectionsDataTable::CollectionInfoPage const *> s_allTitleablePages;

	// all page titles
	std::map<std::string, CollectionsDataTable::CollectionInfoPage const *> s_allPageTitles;

	// all pages by page name
	std::map<std::string, CollectionsDataTable::CollectionInfoPage const *> s_allPagesByName;

	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	// books

	// slots in each book
	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoSlot const *> > s_slotsInBook;

	// collections in each book
	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoCollection const *> > s_collectionsInBook;

	// pages in each book
	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoPage const *> > s_pagesInBook;

	// all books
	std::vector<CollectionsDataTable::CollectionInfoBook const *> s_allBooks;

	// all books by book name
	std::map<std::string, CollectionsDataTable::CollectionInfoBook const *> s_allBooksByName;

	//----------------------------------------------------------------------

	//----------------------------------------------------------------------
	// categories

	// slots in each category
	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoSlot const *> > s_slotsInCategory;

	// slots in each (collection, category)
	std::map<std::pair<std::string, std::string>, std::vector<CollectionsDataTable::CollectionInfoSlot const *> > s_slotsInCategoryByCollection;

	// slots in each (page, category)
	std::map<std::pair<std::string, std::string>, std::vector<CollectionsDataTable::CollectionInfoSlot const *> > s_slotsInCategoryByPage;

	// slots in each (book, category)
	std::map<std::pair<std::string, std::string>, std::vector<CollectionsDataTable::CollectionInfoSlot const *> > s_slotsInCategoryByBook;

	// all categories
	std::set<std::string> s_slotCategories;

	// categories in each collection
	std::map<std::string, std::set<std::string> > s_slotCategoriesByCollection;

	// categories in each page
	std::map<std::string, std::set<std::string> > s_slotCategoriesByPage;

	// categories in each book
	std::map<std::string, std::set<std::string> > s_slotCategoriesByBook;

	//----------------------------------------------------------------------
	
	// the DB can only currently handle this many collections
	//
	// !!!!!!!!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!!!!!
	// ***DO NOT*** change this value unless you know what you
	// are doing, or you will crash the SWG DB Server!!!!!!!!!!!
	// !!!!!!!!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!!!!!!!!!!!
	int const COLLECTIONS_PER_INDEX = 16000;
	int const MAX_COLLECTIONS = COLLECTIONS_PER_INDEX * 2;
}

using namespace CollectionsDataTableNamespace;

// ======================================================================

void CollectionsDataTable::install()
{
	DataTable * table = DataTableManager::getTable(cs_collectionsDataTableName, true);
	if (table)
	{
		int const columnBookName = table->findColumnNumber("bookName");
		int const columnPageName = table->findColumnNumber("pageName");
		int const columnCollectionName = table->findColumnNumber("collectionName");
		int const columnSlotName = table->findColumnNumber("slotName");
		int const columnBeginSlotId = table->findColumnNumber("beginSlotId");
		int const columnEndSlotId = table->findColumnNumber("endSlotId");
		int const columnMaxSlotValue = table->findColumnNumber("maxSlotValue");
		int const columnIcon = table->findColumnNumber("icon");
		int const columnMusic = table->findColumnNumber("music");
		int const columnShowIfNotYetEarned = table->findColumnNumber("showIfNotYetEarned");
		int const columnHidden = table->findColumnNumber("hidden");
		int const columnTitle = table->findColumnNumber("title");
		int const columnNoReward = table->findColumnNumber("noReward");
		int const columnTrackServerFirst = table->findColumnNumber("trackServerFirst");

		// the can be a variable number of "category" columns, as long as the columns
		// are named category1, category2, category3, category4, category5, and so on
		std::vector<int> columnCategory;
		char buffer[128];
		int columnNumber;
		for (int i = 1; i <= 1000000000; ++i)
		{
			snprintf(buffer, sizeof(buffer)-1, "category%d", i);
			buffer[sizeof(buffer)-1] = '\0';

			columnNumber = table->findColumnNumber(buffer);
			if (columnNumber < 0)
				break;

			columnCategory.push_back(columnNumber);
		}

		// the can be a variable number of "prereq" columns, as long as the columns
		// are named prereqSlotName1, prereqSlotName2, prereqSlotName3, prereqSlotName4,
		// prereqSlotName5, and so on
		std::vector<int> columnPrereq;
		for (int i = 1; i <= 1000000000; ++i)
		{
			snprintf(buffer, sizeof(buffer)-1, "prereqSlotName%d", i);
			buffer[sizeof(buffer)-1] = '\0';

			columnNumber = table->findColumnNumber(buffer);
			if (columnNumber < 0)
				break;

			columnPrereq.push_back(columnNumber);
		}

		// the can be a variable number of "alternate title" columns, as long as the columns
		// are named alternateTitle1, alternateTitle2, alternateTitle3, alternateTitle4,
		// alternateTitle5, and so on
		std::vector<int> columnAlternateTitle;
		for (int i = 1; i <= 1000000000; ++i)
		{
			snprintf(buffer, sizeof(buffer)-1, "alternateTitle%d", i);
			buffer[sizeof(buffer)-1] = '\0';

			columnNumber = table->findColumnNumber(buffer);
			if (columnNumber < 0)
				break;

			columnAlternateTitle.push_back(columnNumber);
		}

		FATAL((columnBookName < 0), ("column \"bookName\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnPageName < 0), ("column \"pageName\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnCollectionName < 0), ("column \"collectionName\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnSlotName < 0), ("column \"slotName\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnBeginSlotId < 0), ("column \"beginSlotId\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnEndSlotId < 0), ("column \"endSlotId\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnMaxSlotValue < 0), ("column \"maxSlotValue\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnIcon < 0), ("column \"icon\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnMusic < 0), ("column \"music\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnShowIfNotYetEarned < 0), ("column \"showIfNotYetEarned\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnHidden < 0), ("column \"hidden\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnTitle < 0), ("column \"title\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnNoReward < 0), ("column \"noReward\" not found in %s", cs_collectionsDataTableName));
		FATAL((columnTrackServerFirst < 0), ("column \"trackServerFirst\" not found in %s", cs_collectionsDataTableName));

		CollectionsDataTable::CollectionInfoBook const * currentBook = nullptr;
		CollectionsDataTable::CollectionInfoPage const * currentPage = nullptr;
		CollectionsDataTable::CollectionInfoCollection const * currentCollection = nullptr;
		CollectionsDataTable::CollectionInfoSlot const * currentSlot = nullptr;

		int const numRows = table->getNumRows();
		std::string bookName, pageName, collectionName, slotName, category, prereq, alternateTitle, icon, music;
		std::vector<std::string> categories;
		std::vector<std::string> prereqs;
		std::vector<std::string> titles;
		int beginSlotId, endSlotId, tempBeginSlotId, tempEndSlotId, maxSlotValue;
		unsigned long maxValueForNumBits;
		ShowIfNotYetEarnedType showIfNotYetEarned;
		bool hidden, notifyScriptOnModify, title, noReward, trackServerFirst;
		int tempCount;
		std::map<std::string, int> names;
		std::map<std::string, int> allPrereqs;
		std::map<int, CollectionsDataTable::CollectionInfoSlot const *> allSlotsById;
		for (int i = 0; i < numRows; ++i)
		{
			bookName = table->getStringValue(columnBookName, i);
			pageName = table->getStringValue(columnPageName, i);
			collectionName = table->getStringValue(columnCollectionName, i);
			slotName = table->getStringValue(columnSlotName, i);

			if (bookName.empty() && pageName.empty() && collectionName.empty() && slotName.empty())
				continue;

			// only one of bookName, pageName, collectionName, or slotName can be specified
			// and bookName, pageName, collectionName, and slotName must be unique
			tempCount = 0;
			if (!bookName.empty())
			{
				FATAL((names.count(bookName) >= 1), ("%s, row %d: book name %s already used at row %d (either as a book, page, collection, slot, or alternate title)", cs_collectionsDataTableName, (i+3), bookName.c_str(), names[bookName]));
				names[bookName] = (i+3);
				++tempCount;
			}
			if (!pageName.empty())
			{
				FATAL((names.count(pageName) >= 1), ("%s, row %d: page name %s already used at row %d (either as a book, page, collection, slot, or alternate title)", cs_collectionsDataTableName, (i+3), pageName.c_str(), names[pageName]));
				names[pageName] = (i+3);
				++tempCount;
			}
			if (!collectionName.empty())
			{
				FATAL((names.count(collectionName) >= 1), ("%s, row %d: collection name %s already used at row %d (either as a book, page, collection, slot, or alternate title)", cs_collectionsDataTableName, (i+3), collectionName.c_str(), names[collectionName]));
				names[collectionName] = (i+3);
				++tempCount;
			}
			if (!slotName.empty())
			{
				FATAL((names.count(slotName) >= 1), ("%s, row %d: slot name %s already used at row %d (either as a book, page, collection, slot, or alternate title)", cs_collectionsDataTableName, (i+3), slotName.c_str(), names[slotName]));
				names[slotName] = (i+3);
				++tempCount;
			}

			FATAL((tempCount != 1), ("%s, row %d: only one of bookName, pageName, collectionName, or slotName can be specified", cs_collectionsDataTableName, (i+3)));

			beginSlotId = table->getIntValue(columnBeginSlotId, i);
			endSlotId = table->getIntValue(columnEndSlotId, i);
			maxSlotValue = table->getIntValue(columnMaxSlotValue, i);
			icon = table->getStringValue(columnIcon, i);
			music = table->getStringValue(columnMusic, i);
			showIfNotYetEarned = static_cast<ShowIfNotYetEarnedType>(table->getIntValue(columnShowIfNotYetEarned, i));
			hidden = (table->getIntValue(columnHidden, i) != 0);
			title = (table->getIntValue(columnTitle, i) != 0);
			noReward = (table->getIntValue(columnNoReward, i) != 0);
			trackServerFirst = (table->getIntValue(columnTrackServerFirst, i) != 0);

			// read all alternate titles
			titles.clear();
			if (title)
			{
				if (!pageName.empty())
					titles.push_back(pageName);
				else if (!collectionName.empty())
					titles.push_back(collectionName);
				else if (!slotName.empty())
					titles.push_back(slotName);
			}

			for (std::vector<int>::const_iterator iterColumnAlternateTitle = columnAlternateTitle.begin(); iterColumnAlternateTitle != columnAlternateTitle.end(); ++iterColumnAlternateTitle)
			{
				alternateTitle = table->getStringValue(*iterColumnAlternateTitle, i);

				if (!alternateTitle.empty())
				{
					FATAL((names.count(alternateTitle) >= 1), ("%s, row %d: alternate title %s already used at row %d (either as a book, page, collection, slot, or alternate title)", cs_collectionsDataTableName, (i+3), alternateTitle.c_str(), names[alternateTitle]));
					names[alternateTitle] = (i+3);

					titles.push_back(alternateTitle);
				}
			}

			// start of new book
			if (!bookName.empty())
			{
				// verify previous book, unless it's the first book
				if (currentBook)
				{
					// make sure previous book is not empty
					FATAL(s_pagesInBook[currentBook->name].empty(), ("%s: book %s has no pages", cs_collectionsDataTableName, currentBook->name.c_str()));
					FATAL(!currentPage, ("%s: book %s has no pages", cs_collectionsDataTableName, currentBook->name.c_str()));
					FATAL(s_slotsInBook[currentBook->name].empty(), ("%s: book %s has no slots", cs_collectionsDataTableName, currentBook->name.c_str()));

					// make sure last page of previous book is not empty
					FATAL(s_collectionsInPage[currentPage->name].empty(), ("%s: page %s has no collections", cs_collectionsDataTableName, currentPage->name.c_str()));
					FATAL(!currentCollection, ("%s: page %s has no collections", cs_collectionsDataTableName, currentPage->name.c_str()));
					FATAL(s_slotsInPage[currentPage->name].empty(), ("%s: page %s has no slots", cs_collectionsDataTableName, currentPage->name.c_str()));

					// make sure last collection of last page of previous book is not empty
					FATAL(s_slotsInCollection[currentCollection->name].empty(), ("%s: page %s has empty collection %s", cs_collectionsDataTableName, currentPage->name.c_str(), currentCollection->name.c_str()));
				}

				// start new book
				FATAL(title, ("%s: book %s cannot be \"titleable\")", cs_collectionsDataTableName, bookName.c_str()));
				FATAL(!titles.empty(), ("%s: book %s cannot have any alternate titles (books are not \"titleable\")", cs_collectionsDataTableName, bookName.c_str()));
				currentBook = new CollectionInfoBook(bookName, icon, showIfNotYetEarned, hidden);
				currentPage = nullptr;
				currentCollection = nullptr;
				s_allBooks.push_back(currentBook);
				s_allBooksByName[bookName] = currentBook;
			}
			// start of new page
			else if (!pageName.empty())
			{
				// cannot start page without a book
				FATAL(!currentBook, ("%s, row %d: page %s must be in a book", cs_collectionsDataTableName, (i+3), pageName.c_str()));

				// verify previous page, unless it's the first page
				if (currentPage)
				{
					// make sure previous page is not empty
					FATAL(s_collectionsInPage[currentPage->name].empty(), ("%s: page %s has no collections", cs_collectionsDataTableName, currentPage->name.c_str()));
					FATAL(!currentCollection, ("%s: page %s has no collections", cs_collectionsDataTableName, currentPage->name.c_str()));
					FATAL(s_slotsInPage[currentPage->name].empty(), ("%s: page %s has no slots", cs_collectionsDataTableName, currentPage->name.c_str()));

					// make sure last collection of previous page is not empty
					FATAL(s_slotsInCollection[currentCollection->name].empty(), ("%s: page %s has empty collection %s", cs_collectionsDataTableName, currentPage->name.c_str(), currentCollection->name.c_str()));
				}

				// start new page
				FATAL((!titles.empty() && !title), ("%s: page %s cannot have any alternate titles unless it is defined as \"titleable\")", cs_collectionsDataTableName, pageName.c_str()));
				currentPage = new CollectionInfoPage(pageName, icon, showIfNotYetEarned, hidden, titles, *currentBook);
				currentCollection = nullptr;
				s_pagesInBook[currentBook->name].push_back(currentPage);
				s_allPagesByName[pageName] = currentPage;

				if (!currentPage->titles.empty())
				{
					s_allTitleablePages.push_back(currentPage);

					for (std::vector<std::string>::const_iterator iterTitles = currentPage->titles.begin(); iterTitles != currentPage->titles.end(); ++iterTitles)
						s_allPageTitles[*iterTitles] = currentPage;
				}
			}
			else if (!collectionName.empty())
			{
				// cannot start collection without a page or a book
				FATAL(!currentBook, ("%s, row %d: collection %s must be in a book", cs_collectionsDataTableName, (i+3), collectionName.c_str()));
				FATAL(!currentPage, ("%s, row %d: collection %s must be in a page", cs_collectionsDataTableName, (i+3), collectionName.c_str()));

				// verify previous collection, unless it's the first collection in the page
				if (currentCollection)
				{
					// make sure previous collection is not empty
					FATAL(s_slotsInCollection[currentCollection->name].empty(), ("%s: page %s has empty collection %s", cs_collectionsDataTableName, currentPage->name.c_str(), currentCollection->name.c_str()));
				}

				// read all category for collection
				for (std::vector<int>::const_iterator iterColumnCategory = columnCategory.begin(); iterColumnCategory != columnCategory.end(); ++iterColumnCategory)
				{
					category = table->getStringValue(*iterColumnCategory, i);

					if (!category.empty())
					{
						categories.push_back(category);
					}
				}

				// start new collection
				FATAL((!titles.empty() && !title), ("%s: collection %s cannot have any alternate titles unless it is defined as \"titleable\")", cs_collectionsDataTableName, collectionName.c_str()));
				currentCollection = new CollectionInfoCollection(collectionName, icon, showIfNotYetEarned, hidden, categories, titles, noReward, trackServerFirst, *currentPage);
				s_collectionsInPage[currentPage->name].push_back(currentCollection);
				s_collectionsInBook[currentBook->name].push_back(currentCollection);
				s_allCollectionsByName[collectionName] = currentCollection;

				if (!currentCollection->titles.empty())
				{
					s_allTitleableCollections.push_back(currentCollection);

					for (std::vector<std::string>::const_iterator iterTitles = currentCollection->titles.begin(); iterTitles != currentCollection->titles.end(); ++iterTitles)
						s_allCollectionTitles[*iterTitles] = currentCollection;
				}

				if (currentCollection->trackServerFirst)
					s_allServerFirstCollections[collectionName] = currentCollection;

				categories.clear();
			}
			else
			{
				// cannot have a slot without a collection or a page or a book
				FATAL(!currentBook, ("%s, row %d: slot %s must be in a book", cs_collectionsDataTableName, (i+3), slotName.c_str()));
				FATAL(!currentPage, ("%s, row %d: slot %s must be in a page", cs_collectionsDataTableName, (i+3), slotName.c_str()));
				FATAL(!currentCollection, ("%s, row %d: slot %s must be in a collection", cs_collectionsDataTableName, (i+3), slotName.c_str()));

				// check for valid slot id
				FATAL(((beginSlotId < 0) || (beginSlotId >= MAX_COLLECTIONS)), ("%s, row %d: begin slot id %d must be 0-%d", cs_collectionsDataTableName, (i+3), beginSlotId, (MAX_COLLECTIONS-1)));
				FATAL((((endSlotId < 0) && (endSlotId != -1)) || (endSlotId >= MAX_COLLECTIONS)), ("%s, row %d: end slot id %d must be 0-%d", cs_collectionsDataTableName, (i+3), endSlotId, (MAX_COLLECTIONS-1)));

				// check for valid beginSlotId/endSlotId combination
				FATAL(((endSlotId != -1) && (beginSlotId >= endSlotId)), ("%s, row %d: begin slot id %d must be < end slot id %d", cs_collectionsDataTableName, (i+3), beginSlotId, endSlotId));
				FATAL(((endSlotId != -1) && ((beginSlotId / COLLECTIONS_PER_INDEX) != (endSlotId / COLLECTIONS_PER_INDEX))), ("%s, row %d: counter-type slot cannot span across the 15999-16000 / 31999-32000 / 47999-48000 / 63999-64000 / etc slot id boundary", cs_collectionsDataTableName, (i+3)));

				// max number of bits for a counter-type slot is 32 bits
				FATAL(((endSlotId != -1) && ((endSlotId - beginSlotId) > 31)), ("%s, row %d: counter-type slot uses %d bits which exceeds the limit of 32 bits for counter-type slot", cs_collectionsDataTableName, (i+3), (endSlotId - beginSlotId + 1)));

				// maxSlotValue is only valid if both beginSlotId and endSlotId specified
				FATAL(((endSlotId == -1) && (maxSlotValue != -1)), ("%s, row %d: max slot value %d cannot be specified for a non counter-type slot", cs_collectionsDataTableName, (i+3), maxSlotValue));

				// check for valid maxSlotValue
				FATAL(((maxSlotValue <= 1) && (maxSlotValue != -1)), ("%s, row %d: max slot value %d must be > 1", cs_collectionsDataTableName, (i+3), maxSlotValue));

				// if maxSlotValue specified, make sure there are enough bits allocated to be able to store the value
				maxValueForNumBits = 1;
				if (endSlotId != -1)
				{
					unsigned long const numBits = static_cast<unsigned long>(endSlotId - beginSlotId + 1);
					maxValueForNumBits = (0xffffffff >> (32 - numBits));

					if (maxSlotValue != -1)
						FATAL((maxValueForNumBits < static_cast<unsigned long>(maxSlotValue)), ("%s, row %d: counter-type slot uses %lu bits, which can only hold a max value of %lu, which is less than the specified max value of %d", cs_collectionsDataTableName, (i+3), numBits, maxValueForNumBits, maxSlotValue));
				}

				// read all category for slot
				notifyScriptOnModify = true;
				for (std::vector<int>::const_iterator iterColumnCategory = columnCategory.begin(); iterColumnCategory != columnCategory.end(); ++iterColumnCategory)
				{
					category = table->getStringValue(*iterColumnCategory, i);
				
					if (!category.empty())
					{
						categories.push_back(category);

						if (category == "noScriptNotifyOnModify")
							notifyScriptOnModify = false;
					}
				}

				// read all prereq for slot
				for (std::vector<int>::const_iterator iterColumnPrereq = columnPrereq.begin(); iterColumnPrereq != columnPrereq.end(); ++iterColumnPrereq)
				{
					prereq = table->getStringValue(*iterColumnPrereq, i);

					if (!prereq.empty())
					{
						FATAL((prereq == slotName), ("%s, row %d: slot %s cannot have itself as a prereq", cs_collectionsDataTableName, (i+3), slotName.c_str()));

						prereqs.push_back(prereq);

						if (allPrereqs.count(prereq) < 1)
							allPrereqs[prereq] = (i+3);
					}
				}

				// new slot
				FATAL((!titles.empty() && !title), ("%s: slot %s cannot have any alternate titles unless it is defined as \"titleable\")", cs_collectionsDataTableName, slotName.c_str()));
				currentSlot = new CollectionInfoSlot(slotName, icon, showIfNotYetEarned, hidden, notifyScriptOnModify, (beginSlotId / COLLECTIONS_PER_INDEX), (beginSlotId % COLLECTIONS_PER_INDEX), beginSlotId, ((endSlotId < 0) ? -1 : (endSlotId % COLLECTIONS_PER_INDEX)), ((endSlotId < 0) ? -1 : endSlotId), (((endSlotId < 0) || (maxSlotValue <= 1)) ? 0 : static_cast<unsigned long>(maxSlotValue)), maxValueForNumBits, categories, prereqs, music, titles, *currentCollection);

				// check for duplicate slot id across all collections
				if (endSlotId == -1)
				{
					FATAL((allSlotsById.count(beginSlotId) >= 1), ("%s, row %d: slot id %d already used by slot %s", cs_collectionsDataTableName, (i+3), beginSlotId, allSlotsById[beginSlotId]->name.c_str()));
				}
				else
				{
					for (int j = beginSlotId; j <= endSlotId; ++j)
						FATAL((allSlotsById.count(j) >= 1), ("%s, row %d: slot id %d already used by slot %s", cs_collectionsDataTableName, (i+3), j, allSlotsById[j]->name.c_str()));
				}

				s_slotsInCollection[currentCollection->name].push_back(currentSlot);
				s_slotsInPage[currentPage->name].push_back(currentSlot);
				s_slotsInBook[currentBook->name].push_back(currentSlot);
				s_allSlotsByName[currentSlot->name] = currentSlot;

				if (endSlotId == -1)
				{
					allSlotsById[beginSlotId] = currentSlot;
				}
				else
				{
					for (int j = beginSlotId; j <= endSlotId; ++j)
						allSlotsById[j] = currentSlot;
				}

				if (!currentSlot->titles.empty())
				{
					s_allTitleableSlots.push_back(currentSlot);

					for (std::vector<std::string>::const_iterator iterTitles = currentSlot->titles.begin(); iterTitles != currentSlot->titles.end(); ++iterTitles)
						s_allSlotTitles[*iterTitles] = currentSlot;
				}

				for (std::vector<std::string>::const_iterator iterCategories = categories.begin(); iterCategories != categories.end(); ++iterCategories)
				{
					s_slotsInCategory[*iterCategories].push_back(currentSlot);
					s_slotsInCategoryByBook[std::make_pair(currentBook->name, *iterCategories)].push_back(currentSlot);
					s_slotsInCategoryByPage[std::make_pair(currentPage->name, *iterCategories)].push_back(currentSlot);
					s_slotsInCategoryByCollection[std::make_pair(currentCollection->name, *iterCategories)].push_back(currentSlot);

					IGNORE_RETURN(s_slotCategories.insert(*iterCategories));
					IGNORE_RETURN(s_slotCategoriesByBook[currentBook->name].insert(*iterCategories));
					IGNORE_RETURN(s_slotCategoriesByPage[currentPage->name].insert(*iterCategories));
					IGNORE_RETURN(s_slotCategoriesByCollection[currentCollection->name].insert(*iterCategories));
				}

				currentSlot = nullptr;
				categories.clear();
				prereqs.clear();
			}
		}

		DataTableManager::close(cs_collectionsDataTableName);

		// verify last book
		if (currentBook)
		{
			// make sure last book is not empty
			FATAL(s_pagesInBook[currentBook->name].empty(), ("%s: book %s has no pages", cs_collectionsDataTableName, currentBook->name.c_str()));
			FATAL(!currentPage, ("%s: book %s has no pages", cs_collectionsDataTableName, currentBook->name.c_str()));
			FATAL(s_slotsInBook[currentBook->name].empty(), ("%s: book %s has no slots", cs_collectionsDataTableName, currentBook->name.c_str()));

			// make sure last page of last book is not empty
			FATAL(s_collectionsInPage[currentPage->name].empty(), ("%s: page %s has no collections", cs_collectionsDataTableName, currentPage->name.c_str()));
			FATAL(!currentCollection, ("%s: page %s has no collections", cs_collectionsDataTableName, currentPage->name.c_str()));
			FATAL(s_slotsInPage[currentPage->name].empty(), ("%s: page %s has no slots", cs_collectionsDataTableName, currentPage->name.c_str()));

			// make sure last collection of last page of last book is not empty
			FATAL(s_slotsInCollection[currentCollection->name].empty(), ("%s: page %s has empty collection %s", cs_collectionsDataTableName, currentPage->name.c_str(), currentCollection->name.c_str()));
		}

		// save off all slots ordered by slot ids
		s_allSlotsById.resize(allSlotsById.size(), nullptr);
		beginSlotId = -1;
		for (std::map<int, CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterSlotId = allSlotsById.begin(); iterSlotId != allSlotsById.end(); ++iterSlotId)
		{
			tempBeginSlotId = ((iterSlotId->second->beginSlotId == -1) ? iterSlotId->second->beginSlotId : ((iterSlotId->second->slotIdIndex * COLLECTIONS_PER_INDEX) + iterSlotId->second->beginSlotId));
			FATAL((tempBeginSlotId != iterSlotId->second->absoluteBeginSlotId), ("%s: beginSlotId/absoluteBeginSlotId mismatch for slot %s (%d, %d, %d)", cs_collectionsDataTableName, iterSlotId->second->name.c_str(), iterSlotId->second->slotIdIndex, iterSlotId->second->beginSlotId, iterSlotId->second->absoluteBeginSlotId));

			tempEndSlotId = ((iterSlotId->second->endSlotId == -1) ? iterSlotId->second->endSlotId : ((iterSlotId->second->slotIdIndex * COLLECTIONS_PER_INDEX) + iterSlotId->second->endSlotId));
			FATAL((tempEndSlotId != iterSlotId->second->absoluteEndSlotId), ("%s: endSlotId/absoluteEndSlotId mismatch for slot %s (%d, %d, %d)", cs_collectionsDataTableName, iterSlotId->second->name.c_str(), iterSlotId->second->slotIdIndex, iterSlotId->second->endSlotId, iterSlotId->second->absoluteEndSlotId));

			if (tempEndSlotId == -1)
			{
				FATAL((iterSlotId->first != tempBeginSlotId), ("%s: begin slot id mismatch for slot %s (%d, %d)", cs_collectionsDataTableName, iterSlotId->second->name.c_str(), iterSlotId->first, tempBeginSlotId));
			}
			else
			{
				FATAL(((iterSlotId->first < tempBeginSlotId) || (iterSlotId->first > tempEndSlotId)), ("%s: slot id mismatch for slot %s (%d, %d, %d)", cs_collectionsDataTableName, iterSlotId->second->name.c_str(), iterSlotId->first, tempBeginSlotId, tempEndSlotId));
			}

			// make sure that slot ids start at 0 and there are no "holes" in the slot ids
			if (iterSlotId == allSlotsById.begin())
			{
				FATAL((iterSlotId->first != 0), ("%s: slot id must start at 0", cs_collectionsDataTableName));
			}
			else
			{
				FATAL(((beginSlotId + 1) != iterSlotId->first), ("%s: slot id must be contiguous (there is a \"hole\" between %d and %d)", cs_collectionsDataTableName, beginSlotId, iterSlotId->first));
			}

			beginSlotId = iterSlotId->first;

			if (iterSlotId->first == tempBeginSlotId)
			{
				s_allSlotsById[iterSlotId->first] = iterSlotId->second;
			}
			else
			{
				s_allSlotsById[iterSlotId->first] = nullptr;
			}
		}

		// make sure that all collection slot name specified as prereqs actually exists
		for (std::map<std::string, int>::const_iterator iterAllPrereqs = allPrereqs.begin(); iterAllPrereqs != allPrereqs.end(); ++iterAllPrereqs)
		{
			FATAL((s_allSlotsByName.count(iterAllPrereqs->first) < 1), ("%s, row %d: prereq slot name %s does not exist", cs_collectionsDataTableName, iterAllPrereqs->second, iterAllPrereqs->first.c_str()));
		}

		// set up collection slot prereqs as pointers to the actual prereq slot, for faster access
		for (std::map<std::string, CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterAllSlotsByName = s_allSlotsByName.begin(); iterAllSlotsByName != s_allSlotsByName.end(); ++iterAllSlotsByName)
		{
			CollectionsDataTable::CollectionInfoSlot * slot = const_cast<CollectionsDataTable::CollectionInfoSlot *>(iterAllSlotsByName->second);
			for (std::vector<std::string>::const_iterator iterPrereq = slot->prereqs.begin(); iterPrereq != slot->prereqs.end(); ++iterPrereq)
			{
				CollectionsDataTable::CollectionInfoSlot const * prereqSlot = getSlotByName(*iterPrereq);
				FATAL((!prereqSlot), ("%s: prereq slot name %s does not exist", cs_collectionsDataTableName, iterPrereq->c_str()));
				(const_cast<std::vector<CollectionInfoSlot const *> *>(&slot->prereqsPtr))->push_back(prereqSlot);
			}
		}
	}
	else
	{
		FATAL(true, ("collection datatable %s not found", cs_collectionsDataTableName));
	}

	ExitChain::add(remove, "CollectionsDataTable::remove");
}

//----------------------------------------------------------------------

void CollectionsDataTable::remove()
{
	// free slots
	{
		for (std::map<std::string, CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterSlot = s_allSlotsByName.begin(); iterSlot != s_allSlotsByName.end(); ++iterSlot)
		{
			delete iterSlot->second;
		}
	}

	// free collections
	{
		for (std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoCollection const *> >::const_iterator iterPage = s_collectionsInPage.begin(); iterPage != s_collectionsInPage.end(); ++iterPage)
		{
			for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iterCollection = iterPage->second.begin(); iterCollection != iterPage->second.end(); ++iterCollection)
			{
				delete *iterCollection;
			}
		}

	}

	// free pages
	{
		for (std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoPage const *> >::const_iterator iterBook = s_pagesInBook.begin(); iterBook != s_pagesInBook.end(); ++iterBook)
		{
			for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iterPage = iterBook->second.begin(); iterPage != iterBook->second.end(); ++iterPage)
			{
				delete *iterPage;
			}
		}
	}

	// free books
	{
		for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator iterBook = s_allBooks.begin(); iterBook != s_allBooks.end(); ++iterBook)
		{
			delete *iterBook;
		}
	}

	// slots
	s_allSlotsByName.clear();
	s_allSlotsById.clear();
	s_allTitleableSlots.clear();
	s_allSlotTitles.clear();

	// collections
	s_slotsInCollection.clear();
	s_allTitleableCollections.clear();
	s_allCollectionTitles.clear();
	s_allServerFirstCollections.clear();
	s_allCollectionsByName.clear();

	// pages
	s_slotsInPage.clear();
	s_collectionsInPage.clear();
	s_allTitleablePages.clear();
	s_allPageTitles.clear();
	s_allPagesByName.clear();

	// books
	s_slotsInBook.clear();
	s_collectionsInBook.clear();
	s_pagesInBook.clear();
	s_allBooks.clear();
	s_allBooksByName.clear();

	// categories
	s_slotsInCategory.clear();
	s_slotsInCategoryByCollection.clear();
	s_slotsInCategoryByPage.clear();
	s_slotsInCategoryByBook.clear();
	s_slotCategories.clear();
	s_slotCategoriesByCollection.clear();
	s_slotCategoriesByPage.clear();
	s_slotCategoriesByBook.clear();
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

CollectionsDataTable::CollectionInfoSlot const * CollectionsDataTable::getSlotByName(std::string const & slotName)
{
	std::map<std::string, CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterFind = s_allSlotsByName.find(slotName);
	if (iterFind != s_allSlotsByName.end())
		return iterFind->second;

	return nullptr;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

CollectionsDataTable::CollectionInfoSlot const * CollectionsDataTable::getSlotByBeginSlotId(int slotId)
{
	if ((slotId < 0) || (slotId >= static_cast<int>(s_allSlotsById.size())))
		return nullptr;

	return s_allSlotsById[slotId];
}

//----------------------------------------------------------------------

std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & CollectionsDataTable::getAllTitleableSlots()
{
	return s_allTitleableSlots;
}

//----------------------------------------------------------------------

CollectionsDataTable::CollectionInfoSlot const * CollectionsDataTable::isASlotTitle(std::string const & titleName)
{
	std::map<std::string, CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterFind = s_allSlotTitles.find(titleName);
	if (iterFind != s_allSlotTitles.end())
		return iterFind->second;

	return nullptr;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & CollectionsDataTable::getSlotsInCollection(std::string const & collectionName)
{
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> empty;

	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoSlot const *> >::const_iterator iterFind = s_slotsInCollection.find(collectionName);
	if (iterFind != s_slotsInCollection.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------

std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & CollectionsDataTable::getAllTitleableCollections()
{
	return s_allTitleableCollections;
}

//----------------------------------------------------------------------

CollectionsDataTable::CollectionInfoCollection const * CollectionsDataTable::isACollectionTitle(std::string const & titleName)
{
	std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iterFind = s_allCollectionTitles.find(titleName);
	if (iterFind != s_allCollectionTitles.end())
		return iterFind->second;

	return nullptr;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *> const & CollectionsDataTable::getAllServerFirstCollections()
{
	return s_allServerFirstCollections;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

CollectionsDataTable::CollectionInfoCollection const * CollectionsDataTable::getCollectionByName(std::string const & collectionName)
{
	std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iterFind = s_allCollectionsByName.find(collectionName);
	if (iterFind != s_allCollectionsByName.end())
		return iterFind->second;

	return nullptr;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & CollectionsDataTable::getSlotsInPage(std::string const & pageName)
{
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> empty;

	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoSlot const *> >::const_iterator iterFind = s_slotsInPage.find(pageName);
	if (iterFind != s_slotsInPage.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & CollectionsDataTable::getCollectionsInPage(std::string const & pageName)
{
	static std::vector<CollectionsDataTable::CollectionInfoCollection const *> empty;

	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoCollection const *> >::const_iterator iterFind = s_collectionsInPage.find(pageName);
	if (iterFind != s_collectionsInPage.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------

std::vector<CollectionsDataTable::CollectionInfoPage const *> const & CollectionsDataTable::getAllTitleablePages()
{
	return s_allTitleablePages;
}

//----------------------------------------------------------------------

CollectionsDataTable::CollectionInfoPage const * CollectionsDataTable::isAPageTitle(std::string const & titleName)
{
	std::map<std::string, CollectionsDataTable::CollectionInfoPage const *>::const_iterator iterFind = s_allPageTitles.find(titleName);
	if (iterFind != s_allPageTitles.end())
		return iterFind->second;

	return nullptr;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

CollectionsDataTable::CollectionInfoPage const * CollectionsDataTable::getPageByName(std::string const & pageName)
{
	std::map<std::string, CollectionsDataTable::CollectionInfoPage const *>::const_iterator iterFind = s_allPagesByName.find(pageName);
	if (iterFind != s_allPagesByName.end())
		return iterFind->second;

	return nullptr;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & CollectionsDataTable::getSlotsInBook(std::string const & bookName)
{
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> empty;

	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoSlot const *> >::const_iterator iterFind = s_slotsInBook.find(bookName);
	if (iterFind != s_slotsInBook.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & CollectionsDataTable::getCollectionsInBook(std::string const & bookName)
{
	static std::vector<CollectionsDataTable::CollectionInfoCollection const *> empty;

	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoCollection const *> >::const_iterator iterFind = s_collectionsInBook.find(bookName);
	if (iterFind != s_collectionsInBook.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoPage const *> const & CollectionsDataTable::getPagesInBook(std::string const & bookName)
{
	static std::vector<CollectionsDataTable::CollectionInfoPage const *> empty;

	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoPage const *> >::const_iterator iterFind = s_pagesInBook.find(bookName);
	if (iterFind != s_pagesInBook.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoBook const *> const & CollectionsDataTable::getAllBooks()
{
	return s_allBooks;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

CollectionsDataTable::CollectionInfoBook const * CollectionsDataTable::getBookByName(std::string const & bookName)
{
	std::map<std::string, CollectionsDataTable::CollectionInfoBook const *>::const_iterator iterFind = s_allBooksByName.find(bookName);
	if (iterFind != s_allBooksByName.end())
		return iterFind->second;

	return nullptr;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & CollectionsDataTable::getSlotsInCategory(std::string const & categoryName)
{
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> empty;

	std::map<std::string, std::vector<CollectionsDataTable::CollectionInfoSlot const *> >::const_iterator iterFind = s_slotsInCategory.find(categoryName);
	if (iterFind != s_slotsInCategory.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & CollectionsDataTable::getSlotsInCategoryByCollection(std::string const & collectionName, std::string const & categoryName)
{
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> empty;

	std::map<std::pair<std::string, std::string>, std::vector<CollectionsDataTable::CollectionInfoSlot const *> >::const_iterator iterFind = s_slotsInCategoryByCollection.find(std::make_pair(collectionName, categoryName));
	if (iterFind != s_slotsInCategoryByCollection.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & CollectionsDataTable::getSlotsInCategoryByPage(std::string const & pageName, std::string const & categoryName)
{
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> empty;

	std::map<std::pair<std::string, std::string>, std::vector<CollectionsDataTable::CollectionInfoSlot const *> >::const_iterator iterFind = s_slotsInCategoryByPage.find(std::make_pair(pageName, categoryName));
	if (iterFind != s_slotsInCategoryByPage.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------
// *****NOTE***** the pointers returned by these functions *****ARE NOT!!!*****
// owned by the caller, they are owned/managed/freed by this class

std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & CollectionsDataTable::getSlotsInCategoryByBook(std::string const & bookName, std::string const & categoryName)
{
	static std::vector<CollectionsDataTable::CollectionInfoSlot const *> empty;

	std::map<std::pair<std::string, std::string>, std::vector<CollectionsDataTable::CollectionInfoSlot const *> >::const_iterator iterFind = s_slotsInCategoryByBook.find(std::make_pair(bookName, categoryName));
	if (iterFind != s_slotsInCategoryByBook.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------

std::set<std::string> const & CollectionsDataTable::getAllSlotCategoriesInCollection(std::string const & collectionName)
{
	static std::set<std::string> empty;

	std::map<std::string, std::set<std::string> >::const_iterator iterFind = s_slotCategoriesByCollection.find(collectionName);
	if (iterFind != s_slotCategoriesByCollection.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------

std::set<std::string> const & CollectionsDataTable::getAllSlotCategoriesInPage(std::string const & pageName)
{
	static std::set<std::string> empty;

	std::map<std::string, std::set<std::string> >::const_iterator iterFind = s_slotCategoriesByPage.find(pageName);
	if (iterFind != s_slotCategoriesByPage.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------

std::set<std::string> const & CollectionsDataTable::getAllSlotCategoriesInBook(std::string const & bookName)
{
	static std::set<std::string> empty;

	std::map<std::string, std::set<std::string> >::const_iterator iterFind = s_slotCategoriesByBook.find(bookName);
	if (iterFind != s_slotCategoriesByBook.end())
		return iterFind->second;

	return empty;
}

//----------------------------------------------------------------------

std::set<std::string> const & CollectionsDataTable::getAllSlotCategories()
{
	return s_slotCategories;
}

//----------------------------------------------------------------------

Unicode::String CollectionsDataTable::localizeCollectionName(std::string const & name)
{
	return StringId("collection_n", name).localize();
}

//----------------------------------------------------------------------

Unicode::String CollectionsDataTable::localizeCollectionDescription(std::string const & name)
{
	return StringId("collection_d", name).localize();
}

//----------------------------------------------------------------------

Unicode::String CollectionsDataTable::localizeCollectionTitle(std::string const & name)
{
	return StringId("collection_title", name).localize();
}

//----------------------------------------------------------------------

void CollectionsDataTable::setServerFirstData(std::set<std::pair<std::pair<int32, std::string>, std::pair<NetworkId, Unicode::String> > > const & collectionServerFirst)
{
	// clear all "server first" info
	for (std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = s_allServerFirstCollections.begin(); iter != s_allServerFirstCollections.end(); ++iter)
	{
		time_t * serverFirstClaimTime = const_cast<time_t *>(&(iter->second->serverFirstClaimTime));
		NetworkId * serverFirstClaimantId = const_cast<NetworkId *>(&(iter->second->serverFirstClaimantId));
		Unicode::String * serverFirstClaimantName = const_cast<Unicode::String *>(&(iter->second->serverFirstClaimantName));

		*serverFirstClaimTime = 0;
		*serverFirstClaimantId = NetworkId::cms_invalid;
		serverFirstClaimantName->clear();
	}

	// set all "server first" info
	for (std::set<std::pair<std::pair<int32, std::string>, std::pair<NetworkId, Unicode::String> > >::const_iterator iter2 = collectionServerFirst.begin(); iter2 != collectionServerFirst.end(); ++iter2)
	{
		std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iterFind = s_allServerFirstCollections.find(iter2->first.second);
		if (iterFind != s_allServerFirstCollections.end())
		{
			time_t * serverFirstClaimTime = const_cast<time_t *>(&(iterFind->second->serverFirstClaimTime));
			NetworkId * serverFirstClaimantId = const_cast<NetworkId *>(&(iterFind->second->serverFirstClaimantId));
			Unicode::String * serverFirstClaimantName = const_cast<Unicode::String *>(&(iterFind->second->serverFirstClaimantName));

			*serverFirstClaimTime = static_cast<time_t>(iter2->first.first);
			*serverFirstClaimantId = iter2->second.first;
			*serverFirstClaimantName = iter2->second.second;
		}
	}
}

//----------------------------------------------------------------------

char const * CollectionsDataTable::getShowIfNotYetEarnedTypeString(ShowIfNotYetEarnedType const showIfNotYetEarned)
{
	if (showIfNotYetEarned == SE_gray)
		return "gray";
	else if (showIfNotYetEarned == SE_unknown)
		return "unknown";

	return "none";
}

// ======================================================================
