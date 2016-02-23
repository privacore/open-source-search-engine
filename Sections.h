#ifndef _SECTIONS_H_
#define _SECTIONS_H_

#include "HashTableX.h"
#include "Msg0.h"
#include "IndexList.h"
#include "Bits.h"
#include "Words.h"
#include "Rdb.h"

// KEY:
// ssssssss ssssssss ssssssss ssssssss  s = 48 bit site hash
// ssssssss ssssssss hhhhhhhh hhhhhhhh  h = hash value (32 bits of the 64 bits!)
// hhhhhhhh hhhhhhhh tttttttt dddddddd  t = tag type
// dddddddd dddddddd dddddddd ddddddHD  d = docid

// DATA:
// SSSSSSSS SSSSSSSS SSSSSSSS SSSSSSSS  S = SectionVote::m_score
// NNNNNNNN NNNNNNNN NNNNNNNN NNNNNNNN  N = SectionVote::m_numSampled

// h: hash value. typically the lower 32 bits of the 
//    Section::m_sentenceContentHash64 or the Section::m_contentHash64 vars. we
//    do not need the full 64 bits because we have the 48 bit site hash included
//    to reduce collisions substantially.

// 
// BEGIN SECTION BIT FLAGS (sec_t)
// values for Section::m_flags, of type sec_t
// 

// . these are descriptive flags, they are computed when Sections is set
// . SEC_NOTEXT sections do not vote, i.e. they are not stored in Sectiondb
#define SEC_NOTEXT       0x0001 // implies section has no alnum words

// . Weights.cpp zeroes out the weights for these types of sections
// . is section delimeted by the <script> tag, <marquee> tag, etc.
#define SEC_SCRIPT       0x0008
#define SEC_STYLE        0x0010
#define SEC_SELECT       0x0020
#define SEC_MARQUEE      0x0040
#define SEC_CONTAINER    0x0080

// . in title/header. for gigabits in XmlDoc.cpp
// . is section delemited by <title> or <hN> tags?
#define SEC_IN_TITLE     0x0100
#define SEC_IN_HEADER    0x0200

// used by Events.cpp to indicate if section contains a TimeOfDay ("7 p.m.")
#define SEC_HAS_TOD      0x0400 
#define SEC_HIDDEN       0x0800 // <div style="display: none">
#define SEC_IN_TABLE     0x1000
#define SEC_FAKE         0x2000 // <hr>/<br>/sentence based faux section
#define SEC_NOSCRIPT     0x4000

#define SEC_HEADING_CONTAINER 0x8000

#define SEC_MENU         0x010000
#define SEC_LINK_TEXT    0x020000
#define SEC_MENU_HEADER  0x040000
#define SEC_INPUT_HEADER 0x080000
#define SEC_INPUT_FOOTER 0x100000
#define SEC_HEADING      0x200000

// reasons why a section is not an event
#define SEC_UNBALANCED         0x00400000 // interlaced section/tags
#define SEC_OPEN_ENDED         0x00800000 // no closing tag found
#define SEC_SENTENCE           0x01000000 // made by a sentence?
#define SEC_PLAIN_TEXT         0x02000000
//#define SEC_UNUSED           0x04000000

//#define SEC_UNUSED                0x00008000000LL
//#define SEC_UNUSED                0x00010000000LL
#define SEC_SECOND_TITLE            0x00020000000LL
#define SEC_SPLIT_SENT              0x00040000000LL
//#define SEC_UNUSED                0x00080000000LL

//#define SEC_UNUSED                0x00100000000LL
#define SEC_MENU_SENTENCE           0x00200000000LL
// fix for folkmads.org:
#define SEC_HR_CONTAINER            0x00400000000LL
#define SEC_HAS_DOM                 0x00800000000LL
#define SEC_HAS_DOW                 0x01000000000LL
//#define SEC_UNUSED                0x02000000000LL
//#define SEC_UNUSED                0x04000000000LL
#define SEC_TAIL_CRAP               0x08000000000LL

#define SEC_CONTROL                 0x0000010000000000LL
#define SEC_STRIKE                  0x0000020000000000LL
#define SEC_STRIKE2                 0x0000040000000000LL
#define SEC_HAS_MONTH               0x0000080000000000LL
//#define SEC_UNUSED                0x0000100000000000LL
#define SEC_HASEVENTDOMDOW          0x0000200000000000LL
//#define SEC_UNUSED                0x0000400000000000LL
//#define SEC_UNUSED                0x0000800000000000LL

//#define SEC_UNUSED                0x0001000000000000LL
//#define SEC_UNUSED                0x0002000000000000LL
//#define SEC_UNUSED                0x0004000000000000LL
//#define SEC_UNUSED                0x0008000000000000LL
#define SEC_HASHXPATH               0x0010000000000000LL

// . some random-y numbers for Section::m_baseHash
// . used by splitSection() function
#define BH_BULLET  7845934
#define BH_SENTENCE 4590649
#define BH_IMPLIED  95468323

#define NOINDEXFLAGS (SEC_SCRIPT|SEC_STYLE|SEC_SELECT)

// the section type (bit flag vector for SEC_*) is currently 32 bits
typedef int64_t sec_t;
typedef uint32_t turkbits_t;

// this is only needed for sections, not facets in general i don think.
// facets has the whole QueryTerm::m_facetHashTable array with more info
//
// . for gbfacet:gbxpathsite1234567 posdb query stats compilation to
//   show how many pages duplicate your section's content on your site
//   at the same xpath. the hash of the innerHTML for that xpath is 
//   embedded into the posdb key like a number in a number key, so the
//   wordpos bits etc are sacrificed to hold that 32-bit number.
// . used by XmlDoc::getSectionsWithDupStats() for display in
//   XmlDoc::printRainbowSections()
// . these are in QueryTerm::m_facetStats and computed from
//   QueryTerm::m_facetHashTable
class SectionStats {
 public:
	SectionStats() { reset(); }
	void reset ( ) {
		m_totalMatches  = 0; // posdb key "val" matches ours
		m_totalEntries  = 0; // total posdb keys
		m_numUniqueVals = 0; // # of unique "vals"
		m_totalDocIds   = 0;
	};
	// # of times xpath innerhtml matched ours. 1 count per docid max.
	int64_t m_totalMatches;
	// # of times this xpath occurred. doc can have multiple times.
	int64_t m_totalEntries;
	// # of unique vals this xpath had. doc can have multiple counts.
	int64_t m_numUniqueVals;
	int64_t m_totalDocIds;
};


class Section {
public:

	// . the section immediately containing us
	// . used by Events.cpp to count # of timeofdays in section
	class Section *m_parent;

	// . we are in a linked list of sections
	// . this is how we keep order
	class Section *m_next;
	class Section *m_prev;

	// . if we are an element in a list, what is the list container section
	// . a containing section is a section containing MULTIPLE 
	//   smaller sections
	// . right now we limit such contained elements to text sections only
	// . used to set SEC_HAS_MENUBROTHER flag
	class Section *m_listContainer;

	// the sibling section before/after us. can be NULL.
	class Section *m_prevBrother;
	class Section *m_nextBrother;

	// if we are in a bold section in a sentence section then this
	// will point to the sentence section that contains us. if we already
	// are a sentence section then this points to itself.
	class Section *m_sentenceSection;

	// . set in XmlDoc::getSectionsWithDupStats()
	// . voting info for this section over all indexed pages from this site
	SectionStats m_stats;

	// position of the first and last alnum word contained directly OR
	// indirectly in this section. use -1 if no text contained...
	int32_t m_firstWordPos;
	int32_t m_lastWordPos;

	// alnum positions for words contained directly OR indirectly as well
	int32_t m_alnumPosA;
	int32_t m_alnumPosB;

	// . for sentences that span multiple sections UNEVENLY
	// . see aliconference.com and abqtango.com for this crazy things
	// . for like 99% of all sections these guys equal m_firstWordPos and
	//   m_lastWordPos respectively
	int32_t m_senta;
	int32_t m_sentb;

	class Section *m_prevSent;
	class Section *m_nextSent;

	// hash of this tag's baseHash and all its parents baseHashes combined
	uint32_t  m_tagHash;

	// like above but for turk voting. includes hash of the class tag attr
	// from m_turkBaseHash, whereas m_tagHash uses m_baseHash of parent.
	uint32_t m_turkTagHash32;

	// for debug output display of color coded nested sections
	uint32_t m_colorHash;

	// tagid of this section, 0 means none (like sentence section, etc.)
	nodeid_t m_tagId;

	// usually just the m_tagId, but hashes in the class attributes of
	// div and span tags, etc. to make them unique
	uint32_t  m_baseHash;

	// just hash the "class=" value along with the tagid
	uint32_t m_turkBaseHash;

	// kinda like m_baseHash but for xml tags and only hashes the
	// tag name and none of the fields
	uint32_t  m_xmlNameHash;

	// these deal with enumertated tags and are used by Events.cpp
	int32_t  m_numOccurences;

	// hash of all the alnum words DIRECTLY in this section
	uint64_t  m_contentHash64;

	uint64_t  m_sentenceContentHash64;

	// . used by the SEC_EVENTBROTHER algo in Dates.cpp to detect
	//   [more] or [details] links that indicate distinct items
	// . sometimes the "(more)" link is combined into the last sentence
	//   so we have to treat the last link kinda like its own sentence too!
	uint32_t  m_lastLinkContentHash32;

	// hash of all sentences contained indirectly or directly.
	// uses m_sentenceContentHash64 (for sentences)
	uint64_t m_indirectSentHash64;

	// . range of words in Words class we encompass
	// . m_wordStart and m_wordEnd are the tag word #'s
	// . ACTUALLY it is a half-closed interval [a,b) like all else
	//   so m_b-1 is the word # of the ending tag, BUT split sections
	//   do not include ending tags!!! (i.e. <hr>, <br>, &bull, etc.)
	//   that were made with a call to splitSection()
	int32_t  m_a;//wordStart;
	int32_t  m_b;//wordEnd;

	// . # alnum words only in this and only this section
	// . if we have none, we are SEC_NOTEXT
	int32_t  m_exclusive;

	// our depth. # of tags in the hash
	int32_t  m_depth;

	// container for the #define'd SEC_* values above
	sec_t m_flags;

	char m_used;

	int32_t m_gbFrameNum;

	// do we contain section "arg"?
	bool contains ( class Section *arg ) {
		return ( m_a <= arg->m_a && m_b >= arg->m_b ); };

	// do we contain section "arg"?
	bool strictlyContains ( class Section *arg ) {
		if ( m_a <  arg->m_a && m_b >= arg->m_b ) return true;
		if ( m_a <= arg->m_a && m_b >  arg->m_b ) return true;
		return false;
	};

	// does this section contain the word #a?
	bool contains2 ( int32_t a ) { return ( m_a <= a && m_b > a ); };

	bool isVirtualSection ( ) ;
};



#define SECTIONS_LOCALBUFSIZE 500

#define FMT_HTML   1
#define FMT_PROCOG 2
#define FMT_JSON   3

class Sections {

 public:

	Sections ( ) ;
	void reset() ;
	~Sections ( ) ;

	// . returns false if blocked, true otherwise
	// . returns true and sets g_errno on error
	// . sets m_sections[] array, 1-1 with words array "w"
	bool set(class Words *w, class Bits *bits, class Url *url,
			  int64_t siteHash64, char *coll, int32_t niceness, uint8_t contentType );

	bool verifySections ( ) ;

	int32_t getStoredSize ( ) ;
	static int32_t getStoredSize ( char *p ) ;
	int32_t serialize     ( char *p ) ;

	bool growSections ( );

	bool getSectiondbList ( );
	bool gotSectiondbList ( bool *needsRecall ) ;

	void setNextBrotherPtrs ( bool setContainer ) ;

	// this is used by Events.cpp Section::m_nextSent
	void setNextSentPtrs();

	bool print ( SafeBuf *sbuf ,
		     class HashTableX *pt ,
		     class HashTableX *et ,
		     class HashTableX *st ,
		     class HashTableX *at ,
		     class HashTableX *tt ,
		     //class HashTableX *rt ,
		     class HashTableX *priceTable ) ;

	void printFlags ( class SafeBuf *sbuf , class Section *sn ) ;

	bool printVotingInfoInJSON ( SafeBuf *sb ) ;

	bool print2 ( SafeBuf *sbuf ,
		      int32_t hiPos,
		      int32_t *wposVec,
		      char *densityVec,
		      char *diversityVec,
		      char *wordSpamVec,
		      char *fragVec,
		      char format = FMT_HTML );
	bool printSectionDiv ( class Section *sk , char format = FMT_HTML );
	class SafeBuf *m_sbuf;

	char *getSectionsReply ( int32_t *size );
	char *getSectionsVotes ( int32_t *size );

	bool isHardSection ( class Section *sn );

	bool setMenus ( );

	void setHeader ( int32_t r , class Section *first , sec_t flag ) ;

	bool setHeadingBit ( ) ;

	void setTagHashes ( ) ;

	bool m_alnumPosValid;

	// save it
	class Words *m_words    ;
	class Bits  *m_bits     ;
	class Url   *m_url      ;
	int64_t    m_siteHash64 ;
	char        *m_coll     ;
	int32_t         m_niceness ;
	int32_t         m_cpuNiceness ;	
	uint8_t      m_contentType;

	int32_t *m_wposVec;
	char *m_densityVec;
	char *m_diversityVec;
	char *m_wordSpamVec;
	char *m_fragVec;
	
	// url ends in .rss or .xml ?
	bool  m_isRSSExt;

	bool m_isFacebook   ;
	bool m_isEventBrite ;
	bool m_isStubHub    ;

	Msg0  m_msg0;
	key128_t m_startKey;
	int32_t  m_recall;
	IndexList m_list;
	int64_t m_termId;

	int32_t m_numLineWaiters;
	bool m_waitInLine;
	int32_t m_articleStartWord;
	int32_t m_articleEndWord;
	bool m_hadArticle;
	int32_t m_numInvalids;

	int32_t m_numAlnumWordsInArticle;

	// word #'s (-1 means invalid)
	int32_t m_titleStart;
	int32_t m_titleEnd;
	int32_t m_titleStartAlnumPos;

	// these are 1-1 with the Words::m_words[] array
	class Section **m_sectionPtrs;

	// save this too
	int32_t m_nw ;

	// for caching parition scores
	HashTableX m_ct;

	// allocate m_sections[] buffer
	class Section  *m_sections;
	int32_t            m_numSections;
	int32_t            m_maxNumSections;

	// this holds the Sections instances in a growable array
	SafeBuf m_sectionBuf;

	// this holds ptrs to sections 1-1 with words array, so we can
	// see what section a word is in.
	SafeBuf m_sectionPtrBuf;

	int32_t m_numSentenceSections;

	bool m_isTestColl;

	// assume no malloc
	bool  m_needsFree;
	char  m_localBuf [ SECTIONS_LOCALBUFSIZE ];

	// set a flag
	bool  m_badHtml;

	int64_t  *m_wids;
	int32_t       *m_wlens;
	char      **m_wptrs;
	nodeid_t   *m_tids;

	int32_t       m_hiPos;

	bool addImpliedLists ( ) ;

	bool addSentenceSections ( ) ;

	class Section *insertSubSection ( int32_t a, int32_t b, int32_t newBaseHash ) ;

	class Section *m_rootSection; // the first section, aka m_firstSection
	class Section *m_lastSection;

	class Section *m_lastAdded;

	// kinda like m_rootSection, the first sentence section that occurs
	// in the document, is NULL iff no sentences in document
	class Section *m_firstSent;
	class Section *m_lastSent;

	bool containsTagId ( class Section *si, nodeid_t tagId ) ;

	bool isTagDelimeter ( class Section *si , nodeid_t tagId ) ;
	
	bool isDelimeter ( int32_t i , char *delimeter , int32_t *delimEnd ) {

		// . HACK: special case when delimeter is 0x01 
		// . that means we are back-to-back br tags
		if ( delimeter == (char *)0x01 ) {
			// must be a br tag
			if ( m_tids[i] != TAG_BR ) return false;
			// assume that
			int32_t k = i + 1;
			// bad if end
			if ( k >= m_nw ) return false;
			// bad if a wid
			if ( m_wids[k] ) return false;
			// inc if punct
			if ( ! m_tids[k] ) k++;
			// bad if end
			if ( k >= m_nw ) return false;
			// must be another br tag
			if ( m_tids[k] != TAG_BR ) return false;
			// mark as end i guess
			*delimEnd = k + 1;
			return true;
		}

		// no word is a delimeter
		if ( m_wids[i] ) return false;
		// tags "<hr" and "<br"
		if ( m_wptrs[i][0] == delimeter[0] &&
		     m_wptrs[i][1] == delimeter[1] &&
		     m_wptrs[i][2] == delimeter[2] )
			return true;
		// if no match above, forget it
		if ( m_tids[i] ) return false;
		// otherwise, we are a punctuation "word"
		// the bullet is 3 bytes long
		if ( m_wlens[i] < 3 ) return false;
		// if not a bullet, skip it (&bull)
		char *p    = m_wptrs[i];
		char *pend = p + m_wlens[i];
		for ( ; p < pend ; p++ ) {
			if ( p[0] != delimeter[0] ) continue;
			if ( p[1] != delimeter[1] ) continue;
			if ( p[2] != delimeter[2] ) continue;
			return true;
		}
		return false;
	}
		

};

// convert sectionType to a string
char *getSectionTypeAsStr ( int32_t sectionType );

// hash of the last 3 parent tagids
//uint32_t getSectionContentTagHash3 ( class Section *sn ) ;

// only allow this many urls per site to add sectiondb info
#define MAX_SITE_VOTERS 32

// . the key in sectiondb is basically the Section::m_tagHash 
//   (with a docId) and the data portion of the Rdb record is this SectionVote
// . the Sections::m_nsvt and m_osvt hash tables contain SectionVotes
//   as their data value and use an tagHash key as well
class SectionVote {
public:
	// . seems like addVote*() always uses a score of 1.0
	// . seems to be a weight used when setting Section::m_votesFor[Not]Dup
	// . not sure if we really use this now
	float m_score;
	// . how many times does this tagHash occur in this doc?
	// . this eliminates the need for the SV_UNIQUE section type
	// . this is not used for tags of type contenthash or taghash
	// . seems like pastdate and futuredate and eurdatefmt 
	//   are the only vote types that actually really use this...
	float m_numSampled;
};

//
// BEGIN SECTION TYPES
//

// . these are the core section types
// . these are not to be confused with the section bit flags below
// . we put these into sectiondb in the form of a SectionVote
// . the SectionVote is the data portion of the rdb record, and the key
//   of the rdb record contains the url site hash and the section m_tagHash
// . in this way, a page can vote on what type of section a tag hash describes
//#define SV_TEXTY          1 // section has mostly non-hypertext words
#define SV_CLOCK          2 // DateParse2.cpp. section contains a clock
#define SV_EURDATEFMT     3 // DateParse2.cpp. contains european date fmt
#define SV_EVENT          4 // used in Events.cpp to indicate event container
#define SV_ADDRESS        5 // used in Events.cpp to indicate address container

// . HACK: the "date" is not the enum tag hash, but is the tagPairHash for this
// . every doc has just one of these describing the entire layout of the page
// . basically looking for these is same as doing a gbtaghash: query
#define SV_TAGPAIRHASH   20 
// . HACK: the "date" is not the enum tag hash, but is the contentHash!
// . this allows us to detect a duplicate section even though the layout
//   of the web page is not quite the same, but is from the same site
#define SV_TAGCONTENTHASH   21 

// now Dates.cpp sets these too
#define SV_FUTURE_DATE   24
#define SV_PAST_DATE     25
#define SV_CURRENT_DATE  26
#define SV_SITE_VOTER    29
#define SV_TURKTAGHASH   30

#endif
