// Suggested 3-tier filtering approach
//. FILE TIER (Metadata) - O(n)
	//├─ File type filter (.txt, .pdf, etc.)
	//├─ Date range filter
	//└─ Size filter (~1ms per file)

//2. INDEX TIER (Tokenized Search) - O(log n)
	//├─ Inverted Index lookup
	//├─ Keyword matching
	//└─ Pre-filter candidates (~10ms)

//3. CONTENT TIER (Fingerprinting) - O(m*n)
	//├─ Levenshtein distance
	//├─ Normalize & compare
	//└─ Sort by similarity (~100ms for 1GB)
	// blueprint following for this program.
	 
	 //performance
	 //Test Cases:
//├─ Small Dataset (10 files, 1MB total)
//│  └─ Expected: <100ms
//├─ Medium Dataset (1000 files, 500MB)
//│  └─ Expected: <2 seconds
//├─ Large Dataset (10000 files, 5GB)
//│  └─ Expected: <10 seconds
//└─ Stress Test (100 concurrent searches)
	//└─ Expected: No crashes, memory stable
	
	//accuracy testing:
	//Test Cases:
//├─ Exact match detection (100% expected)
//├─ Typo tolerance (80%+ similarity)
//├─ Leet speak normalization (4p@ss = "4pass")
//├─ Case insensitivity
//├─ Punctuation handling
//└─ Unicode/special characters

  //security_testing:
//├─ Path traversal prevention (../ blocking)
//├─ SQL injection prevention (if using DB)
//├─ File access permissions validation
//├─ Sensitive data masking in results
//└─ Clipboard content encryption (for DLP)

  //ui/ux testing:
//├─ Search input validation
//├─ Error messaging (no results, access denied)
//├─ Results pagination/infinite scroll
//├─ Export results to CSV/JSON
//├─ Real-time search preview
//└─ Responsiveness (<100ms UI updates)


//# thoughts on implementation:

//Best Algorithm Choice:

//Short-term: Stick with Levenshtein + Inverted Index (fast filtering)
//Long-term: Migrate to BM25 for larger datasets
//GUI Technology:

//Qt (C++) — Native integration with your C++ backend ⭐
//Electron (JavaScript) — Easier UI, but slower
//PyQt (Python) — Quick prototyping
//Caching Strategy:

//Cache file metadata (names, dates, sizes)
//Pre-build inverted index on startup
//Cache previous search results
