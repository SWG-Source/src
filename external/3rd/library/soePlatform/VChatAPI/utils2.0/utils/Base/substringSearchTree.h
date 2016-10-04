#ifndef _SUBSTRING_SEARCH_TREE_H_
#define _SUBSTRING_SEARCH_TREE_H_

#include <algorithm>
#include <list>
#include <vector>

template<typename letter_t, typename data_t>
class stringTree
{
public:
	typedef std::basic_string<letter_t> string_t;

	stringTree();
	stringTree(const stringTree & src);
	virtual ~stringTree();

	stringTree & operator=(const stringTree & rhs);

private:
	class letterNode;
	typedef typename std::vector<const letterNode *> nodeVector_t;

public:
	class const_iterator
	{
	public:
		friend class stringTree;

		const_iterator() { }
		virtual ~const_iterator() { }

		void getKey(string_t & key) const;
		inline string_t key() const { string_t value; getKey(value); return value; }

		const_iterator & operator=(const const_iterator & rhs) { mBranch = rhs.mBranch; return *this; }

		const_iterator & operator++();
		const_iterator operator++(int) { const_iterator tmp = *this; ++*this; return tmp; }
		const_iterator & operator--();
		const_iterator operator--(int) { const_iterator tmp = *this; --*this; return tmp; }
		bool operator==(const const_iterator & rhs) const;
		bool operator!=(const const_iterator & rhs) const;

		const data_t & operator*() const;
		const data_t * operator->() const;

	protected:
		bool advance();
		bool retreat();

		nodeVector_t mBranch;
	};

	class iterator : public const_iterator
	{
	public:
		friend class stringTree;

		iterator() { }
		virtual ~iterator() { }

		iterator & operator=(const iterator & rhs) { *((const_iterator *)this) = rhs; return *this; }

		data_t & operator*() const;
		data_t * operator->() const;
	};

	inline const iterator & begin() { if(mBeginDirty) { setBegin(); } return mBegin; }
	inline const const_iterator & begin() const { if(mBeginDirty) { setBegin(); } return mBegin; }
	inline const iterator & end() { return mEnd; }
	inline const const_iterator & end() const { return mEnd; }

	template<typename iter_t>
	iterator find(const iter_t & start, const iter_t & stop);

	template<typename iter_t>
	const_iterator find(const iter_t & start, const iter_t & stop) const;

	template<typename container_t>
	inline iterator find(const container_t & key) { return find(key.begin(), key.end()); }

	template<typename container_t>
	inline const_iterator find(const container_t & key) const { return find(key.begin(), key.end()); }

	template<typename iter_t>
	bool seek(iter_t & start, const iter_t & stop, iterator & position);

	template<typename iter_t>
	bool seek(iter_t & start, const iter_t & stop, const_iterator & position) const;

	template<typename iter_t>
	bool search(iter_t & start, iter_t & middle, const iter_t & stop, typename stringTree::const_iterator & position) const;

	template<typename iter_t>
	bool insert(const iter_t & start, const iter_t & stop, data_t data);
	inline bool insert(const string_t & key, data_t data) { return insert(key.begin(), key.end(), data); }

	data_t & operator[](const string_t & key);

	template<typename iter_t>
	bool erase(const iter_t & start, const iter_t & stop);
	bool erase(const iterator & position);
	inline bool erase(const string_t & key) { return erase(key.begin(), key.end()); }

	void clear();

	size_t merge(const const_iterator & start, const const_iterator & stop);
	inline size_t merge(const stringTree & src) { return merge(src.begin(), src.end()); }
	
	size_t size() const { return mSize; }
	bool empty() const { return (mSize == 0); }
	size_t nodeCount() const { return mNodeCount; }
	size_t depth() const { return mDepthVector.size(); }
	size_t width(size_t level) const { return mDepthVector[level]; }

	template<typename iter_t>
	class finder
	{
	public:
		finder(const stringTree & dictionary, const iter_t & start, const iter_t & end)
			: mDictionary(dictionary)
			, mTextStart(start)
			, mTextMiddle(start)
			, mTextEnd(end)
		{ }

		template<typename container_t>
		finder(const stringTree & dictionary, const container_t & text)
			: mDictionary(dictionary)
			, mTextStart(text.begin())
			, mTextMiddle(text.begin())
			, mTextEnd(text.end())
		{ }

		virtual ~finder() { }

		bool next();
		const data_t * data() const;

		inline const iter_t & start() const { return mTextStart; }
		inline const iter_t & middle() const { return mTextMiddle; }
		inline const iter_t & end() const { return mTextEnd; }
		inline const iter_t & match() const { return mMatchStart; }

	private:
		iter_t mTextStart;
		iter_t mTextMiddle;
		iter_t mTextEnd;
		iter_t mMatchStart;
		const stringTree & mDictionary;
		typename stringTree::const_iterator mDictionaryIter;
	};

private:
	class letterNode
	{
	public:
		letterNode() : mpData(nullptr), mpChildren(nullptr), mNumChildren(0) { }
		letterNode(const letter_t & letter) : mLetter(letter), mpData(nullptr), mpChildren(nullptr), mNumChildren(0) { }
		~letterNode();

		inline bool hasData() const { return (mpData != nullptr); }
		inline data_t & getData() { return *mpData; }
		inline const data_t & getData() const { return *mpData; }
		inline void setData(const data_t & data) { if (mpData) { *mpData = data; } else { mpData = new data_t(data); } }
		inline void removeData() { delete mpData; mpData = nullptr; }

		letterNode * get(letter_t index) const;
		bool put(letter_t index, letterNode ** tree);
		bool take(letter_t index, bool deallocate = true);

		letterNode * firstChild() const;
		letterNode * lastChild() const;
		letterNode * nextChild(const letterNode * child) const;
		letterNode * previousChild(const letterNode * child) const;

		inline bool operator<(const letter_t & rhs) const { return mLetter < rhs; }
		friend inline bool operator<(const const_iterator & lhs, const letter_t & rhs) { return lhs.mLetter < rhs; }
		inline bool operator==(const letter_t & rhs) const { return mLetter == rhs; }
		friend inline bool operator==(const const_iterator & lhs, const letter_t & rhs) { return lhs.mLetter == rhs; }

		inline letter_t & letter() { return mLetter; }
		inline const letter_t & letter() const { return mLetter; }
		inline bool hasNoChildren() const { return (mpChildren == nullptr) || (mNumChildren == 0); }

	private:
		letter_t mLetter;
		data_t * mpData;
		letterNode * mpChildren;
		size_t mNumChildren;
	};

private:
	void setBegin() const;

	size_t mSize;
	size_t mNodeCount;
	std::vector<size_t> mDepthVector;
	letterNode mRoot;
	mutable iterator mBegin;
	mutable iterator mEnd;
	mutable bool mBeginDirty;
	bool mIsClearing;
};

template<typename letter_t, typename data_t>
stringTree<letter_t, data_t>::stringTree()
	: mSize(0)
	, mNodeCount(0)
	, mBeginDirty(true)
	, mIsClearing(false)
{
	setBegin();
	mEnd.mBranch.push_back(&mRoot);
}

template<typename letter_t, typename data_t>
stringTree<letter_t, data_t>::stringTree(const stringTree & src)
	: mSize(0)
	, mNodeCount(0)
	, mBeginDirty(true)
	, mIsClearing(false)
{
	setBegin();
	mEnd.mBranch.push_back(&mRoot);
	merge(src);
}

template<typename letter_t, typename data_t>
stringTree<letter_t, data_t> & stringTree<letter_t, data_t>::operator=(const stringTree<letter_t, data_t> & rhs)
{
	clear();
	merge(rhs);

	return *this;
}

template<typename letter_t, typename data_t>
stringTree<letter_t, data_t>::~stringTree()
{
	clear();
}

template<typename letter_t, typename data_t>
template<typename iter_t>
bool stringTree<letter_t, data_t>::search(iter_t & start, iter_t & middle, const iter_t & stop, typename stringTree::const_iterator & position) const
{
	if (seek(middle, stop, position)) {
		start++;
	} else {
		for (;
			(position == end()) && (start!= stop);
			start++)
		{
			position = end();
			middle = start;
			seek(middle, stop, position);
		}
	}

	return (position != end());
}

template<typename letter_t, typename data_t>
template<typename iter_t>
bool stringTree<letter_t, data_t>::seek(iter_t & start, const iter_t & stop, typename stringTree::iterator & position)
{
	bool found = false;

	if (position.mBranch.empty()) {
		position.mBranch.push_back(&mRoot);
	}

	const letterNode * pnode = (start != stop) ? position.mBranch.back() : nullptr;

	for	(; (start != stop) && ((pnode = pnode->get(*start)) != nullptr); start++)
	{
		position.mBranch.push_back(pnode);
		if (pnode->hasData()) {
			start++;
			break;
		}
	}
	found = pnode && pnode->hasData();
	if (!found) { position = end(); }

	return found;
}

template<typename letter_t, typename data_t>
template<typename iter_t>
bool stringTree<letter_t, data_t>::seek(iter_t & start, const iter_t & stop, typename stringTree::const_iterator & position) const
{
	bool found = false;

	if (position.mBranch.empty()) {
		position.mBranch.push_back(&mRoot);
	}

	const letterNode * pnode = (start != stop) ? position.mBranch.back() : nullptr;

	for	(; (start != stop) && ((pnode = pnode->get(*start)) != nullptr); start++)
	{
		position.mBranch.push_back(pnode);
		if (pnode->hasData()) {
			start++;
			break;
		}
	}
	found = pnode && pnode->hasData();
	if (!found) { position = end(); }

	return found;
}

template<typename letter_t, typename data_t>
template<typename iter_t>
typename stringTree<letter_t, data_t>::iterator stringTree<letter_t, data_t>::find(const iter_t & start, const iter_t & stop)
{
	iter_t iter = start;
	iterator pos;

	for (seek(iter, stop, pos);
             (iter != stop) && (pos != end());
             seek(iter, stop, pos))
	{ }

	return pos;
}

template<typename letter_t, typename data_t>
template<typename iter_t>
typename stringTree<letter_t, data_t>::const_iterator stringTree<letter_t, data_t>::find(const iter_t & start, const iter_t & stop) const
{
	iter_t iter = start;
	const_iterator pos;

	for (seek(iter, stop, pos);
             (iter != stop) && (pos != end());
             seek(iter, stop, pos))
	{ }

	return pos;
}

template<typename letter_t, typename data_t>
template<typename iter_t>
bool stringTree<letter_t, data_t>::insert(const iter_t & start, const iter_t & stop, data_t data)
{
	letterNode * pnode = &mRoot;
	bool added = false;
	size_t depth = 0;

	for (iter_t iter = start; iter != stop; iter++, depth++)
	{
		bool justAdded = pnode->put(*iter, &pnode);

		if (justAdded) {
			if (mDepthVector.size() <= depth) {
				mDepthVector.push_back(0);
			}
			mDepthVector[depth]++;
			mNodeCount++;
		}
		added = added || justAdded;
	}
	added = added || !pnode->hasData();
	if (added) { mSize++; mBeginDirty = true; }
	pnode->setData(data);

	return added;
}

template<typename letter_t, typename data_t>
template<typename iter_t>
bool stringTree<letter_t, data_t>::erase(const iter_t & start, const iter_t & stop)
{
	iterator position = find(start, stop);
	bool removed = erase(position);

	return removed;
}

template<typename letter_t, typename data_t>
bool stringTree<letter_t, data_t>::erase(const iterator & position)
{
	bool removed = false;
	size_t depth = position.mBranch.size();

	if (depth > 1) {
		const nodeVector_t & trace = position.mBranch;
		typename nodeVector_t::const_reverse_iterator trIter;
		letter_t lastLetter;
		bool lastWasEmpty = false;

		depth--;
		const_cast<letterNode *>(position.mBranch.back())->removeData();
		for (trIter = trace.rbegin(); trIter != trace.rend(); trIter++, depth--)
		{
			if (lastWasEmpty) {
				const_cast<letterNode *>(*trIter)->take(lastLetter, !mIsClearing);
				mNodeCount--;
				mDepthVector[depth]--;
				if (mDepthVector[depth] == 0) {
					mDepthVector.pop_back();
				}
			}
			lastLetter = (*trIter)->letter();
			lastWasEmpty = (*trIter)->hasNoChildren() && !(*trIter)->hasData();
		}

		removed = true;
	}
	if (removed) { mSize--; mBeginDirty = true; }

	return removed;
}

template<typename letter_t, typename data_t>
data_t & stringTree<letter_t, data_t>::operator[](const string_t & key)
{
	iterator iter = find(key);

	if (iter == end()) {
		insert(key, data_t());
		iter = find(key);
	}

	return *iter;
}

template<typename letter_t, typename data_t>
stringTree<letter_t, data_t>::letterNode::~letterNode()
{
	/*
		does not free any memory; that is left up to the owner
	*/
}

template<typename letter_t, typename data_t>
typename stringTree<letter_t, data_t>::letterNode * stringTree<letter_t, data_t>::letterNode::get(letter_t index) const
{
	letterNode * tree = nullptr;
	letterNode * end = mpChildren + mNumChildren;
	letterNode * place = std::lower_bound<letterNode *>(mpChildren, mpChildren + mNumChildren, index);

	if (place && (place < end) && (place->mLetter == index)) {
		tree = place;
	}

	return tree;
}

template<typename letter_t, typename data_t>
bool stringTree<letter_t, data_t>::letterNode::put(letter_t index, typename stringTree::letterNode ** tree)
{
	letterNode * end = mpChildren + mNumChildren;
	letterNode * place = std::lower_bound<letterNode *>(mpChildren, mpChildren + mNumChildren, index);
	bool added = false;

	if (place && (place < end) && (place->mLetter == index)) {
		*tree = place;
	} else {
		mNumChildren++;

		letterNode * children = new letterNode[mNumChildren];
		size_t before = place - mpChildren;
		size_t after = end - place;
		letterNode * post = children + before + 1;

		memcpy(children, mpChildren, before * sizeof(letterNode));
		memcpy(post, place, after * sizeof(letterNode));

		place = children + before;
		place->mLetter = index;
		delete [] mpChildren;
		mpChildren = children;

		*tree = place;
		added = true;
	}

	return added;
}

template<typename letter_t, typename data_t>
bool stringTree<letter_t, data_t>::letterNode::take(letter_t index, bool deallocate)
{
	letterNode * end = mpChildren + mNumChildren;
	letterNode * place = std::lower_bound<letterNode *>(mpChildren, end, index);
	bool removed = false;

	if (place && (place < end) && (place->mLetter == index)) {
		mNumChildren--;
		if (!mNumChildren) { deallocate = true; }

		letterNode * children = nullptr;

		if (deallocate) {
			children = mNumChildren ? new letterNode[mNumChildren] : nullptr;
		} else {
			children = mpChildren;	
		}

		size_t before = place - mpChildren;
		size_t after = end - place - 1;
		letterNode * post = children + before;

		if (deallocate) { memcpy(children, mpChildren, before * sizeof(letterNode)); }
		memcpy(post, place + 1, after * sizeof(letterNode));

		if (deallocate) { delete [] mpChildren; }
		mpChildren = children;

		removed = true;
	}

	return removed;
}

template<typename letter_t, typename data_t>
void stringTree<letter_t, data_t>::const_iterator::getKey(string_t & key) const
{
	typename nodeVector_t::const_iterator iter = mBranch.begin();
	if (iter != mBranch.end()) {
		// skip the root
		iter++;
	}
	for (; iter != mBranch.end(); iter++)
	{
		key.append(1, (*iter)->letter());
	}
}

template<typename letter_t, typename data_t>
typename stringTree<letter_t, data_t>::letterNode * stringTree<letter_t, data_t>::letterNode::firstChild() const
{
	letterNode * child = nullptr;

	if (mpChildren) {
		child = mpChildren;
	}

	return child;
}

template<typename letter_t, typename data_t>
typename stringTree<letter_t, data_t>::letterNode * stringTree<letter_t, data_t>::letterNode::lastChild() const
{
	letterNode * child = nullptr;

	if (mpChildren) {
		child = mpChildren + mNumChildren - 1;
	}

	return child;
}

template<typename letter_t, typename data_t>
typename stringTree<letter_t, data_t>::letterNode * stringTree<letter_t, data_t>::letterNode::nextChild(const letterNode * child) const
{
	letterNode * next = nullptr;

	if (mpChildren) {
		size_t index = child - mpChildren;
		
		index++;
		if (index < mNumChildren) {
			next = mpChildren + index;
		}
	}

	return next;
}

template<typename letter_t, typename data_t>
typename stringTree<letter_t, data_t>::letterNode * stringTree<letter_t, data_t>::letterNode::previousChild(const letterNode * child) const
{
	letterNode * previous = nullptr;

	if (mpChildren) {
		size_t index = child - mpChildren;

		if (index > 0) {
			index--;
			previous = mpChildren + index;
		}
	}

	return previous;
}

template<typename letter_t, typename data_t>
typename stringTree<letter_t, data_t>::const_iterator& stringTree<letter_t, data_t>::const_iterator::operator++()
{
	while (advance() && !mBranch.back()->hasData())
	{ }

	if (!mBranch.back()->hasData()) {
		mBranch.resize(1);
	}

	return *this;
}

template<typename letter_t, typename data_t>
typename stringTree<letter_t, data_t>::const_iterator & stringTree<letter_t, data_t>::const_iterator::operator--()
{
	while (retreat() &&
		!mBranch.back()->hasData() &&
		(mBranch.size() > 1))
	{ }

	if (!mBranch.back()->hasData()) {
		mBranch.resize(1);
	}

	return *this;
}

template<typename letter_t, typename data_t>
bool stringTree<letter_t, data_t>::const_iterator::advance()
{
	bool wentForward = false;
	const letterNode * terminal = nullptr;
	const letterNode * penultimate = nullptr;
	const letterNode * next = nullptr;

	if (mBranch.back()->hasNoChildren()) {
		// go forward
		while ((mBranch.size() >= 2) && (next == nullptr))
		{
			penultimate = mBranch[mBranch.size() - 2];
			terminal = mBranch[mBranch.size() - 1];
			next = penultimate->nextChild(terminal);
			mBranch.pop_back();
		}
	} else {
		// go down
		terminal = mBranch[mBranch.size() - 1];
		next = terminal->firstChild();
	}

	if (next) {
		mBranch.push_back(next);
		wentForward = true;
	}

	return wentForward;
}

template<typename letter_t, typename data_t>
bool stringTree<letter_t, data_t>::const_iterator::retreat()
{
	bool wentBack = false;
	const letterNode * terminal = nullptr;
	const letterNode * penultimate = nullptr;
	const letterNode * next = nullptr;

	if (mBranch.size() >= 2) {
		// go backwards
		penultimate = mBranch[mBranch.size() - 2];
		terminal = mBranch[mBranch.size() - 1];
		next = penultimate->previousChild(terminal);
		mBranch.pop_back();
		wentBack = true;
	} else {
		next = mBranch.back();
		mBranch.pop_back();
	}
	// go to leaf
	for (; next != nullptr; next = next->lastChild())
	{
		mBranch.push_back(next);
		if (next->hasData() && next->hasNoChildren()) {
			wentBack = true;
			break;
		}
	}

	return wentBack;
}

template<typename letter_t, typename data_t>
bool stringTree<letter_t, data_t>::const_iterator::operator==(const typename stringTree<letter_t, data_t>::const_iterator & rhs) const
{
	bool isEqual = (mBranch.size() == rhs.mBranch.size()) && 
					(mBranch.empty() || (mBranch.back() == rhs.mBranch.back()));
	return isEqual;
}

template<typename letter_t, typename data_t>
bool stringTree<letter_t, data_t>::const_iterator::operator!=(const typename stringTree<letter_t, data_t>::const_iterator & rhs) const
{
	return (mBranch != rhs.mBranch);
}

template<typename letter_t, typename data_t>
const data_t & stringTree<letter_t, data_t>::const_iterator::operator*() const
{
	return mBranch.back()->getData();
}

template<typename letter_t, typename data_t>
const data_t * stringTree<letter_t, data_t>::const_iterator::operator->() const
{
	return &(mBranch.back()->getData());
}

template<typename letter_t, typename data_t>
data_t & stringTree<letter_t, data_t>::iterator::operator*() const
{
	return ((data_t &)**(const_iterator *)this);
}

template<typename letter_t, typename data_t>
data_t * stringTree<letter_t, data_t>::iterator::operator->() const
{
	return (&**this);
}

template<typename letter_t, typename data_t>
void stringTree<letter_t, data_t>::setBegin() const
{
	mBegin.mBranch.clear();
	const letterNode * leaf = nullptr;

	for (leaf = &mRoot; leaf != nullptr; leaf = leaf->firstChild())
	{
		mBegin.mBranch.push_back(leaf);
		if (leaf->hasData()) {
			break;
		}
	}
	if (!mBegin.mBranch.back()->hasData()) {
		mBegin.mBranch.resize(1);
	}
	mBeginDirty = false;
}

template<typename letter_t, typename data_t>
void stringTree<letter_t, data_t>::clear()
{
	mIsClearing = true;
	while (!empty())
	{
		setBegin();
		erase(mBegin);
	}
	mIsClearing = false;
}

template<typename letter_t, typename data_t>
size_t stringTree<letter_t, data_t>::merge(const const_iterator & start, const const_iterator & stop)
{
	const_iterator iter;
	size_t numberInserted = 0;

	for (iter = start; iter != stop; iter++)
	{
		if (insert(iter.key(), *iter)) {
			numberInserted++;
		}
	}
}

template<typename letter_t, typename data_t>
template<typename iter_t>
bool stringTree<letter_t, data_t>::finder<iter_t>::next()
{
	bool found = mDictionary.search(mTextStart, mTextMiddle, mTextEnd, mDictionaryIter);

	if (found) {
		mMatchStart = mTextStart - 1;
	}

	return found;
}

template<typename letter_t, typename data_t>
template<typename iter_t>
const data_t * stringTree<letter_t, data_t>::finder<iter_t>::data() const
{
	const data_t * pData = nullptr;

	if (mDictionaryIter != mDictionary.end()) {
		pData = &(*mDictionaryIter);
	}

	return pData;
}

#endif // _SUBSTRING_SEARCH_TREE_H_

