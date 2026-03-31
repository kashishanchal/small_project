# Software Requirements Specification (SRS)
## GUI Dashboard for Content Fingerprinting & File Search

**Version:** 1.0  
**Date:** March 31, 2026  
**Project:** DLP Clipboard Monitor - GUI Dashboard Enhancement  
**Author:** Kashish  

---

## 1. Executive Summary

This document specifies the requirements for a GUI Dashboard that extends the existing DLP Clipboard Monitor with interactive search, filtering, and content fingerprinting capabilities. Users can search for sensitive text across protected file directories with fuzzy matching and receive detailed results with file metadata.

---

## 2. Scope

### 2.1 In Scope
- Interactive GUI search interface
- Text search with Levenshtein distance matching
- File type filtering (.txt, .pdf, .docx, .json, .xml)
- Date range filtering
- Custom directory path selection
- Similarity threshold configuration
- Results display with metadata (filename, path, modification date, size, match%)
- Export results functionality
- Search history

### 2.2 Out of Scope
- PDF/DOCX full-text parsing (Phase 2)
- Cloud storage integration
- Multi-user authentication
- Real-time clipboard monitoring UI (existing shell script only)

---

## 3. Functional Requirements

### 3.1 User Interface
**FR-UI-1:** Dashboard shall display main search panel with:
- Text input field for search query (required)
- File type filter dropdown (multi-select, optional)
- Date range picker (optional)
- Directory path input (optional, defaults to ~/Desktop/protected)
- Similarity threshold slider (0.0-1.0, default 0.01)
- Search button

**FR-UI-2:** Results panel shall display:
- Tabular results with columns: Filename | Path | Match Score | File Size | Last Modified
- Sortable columns (ascending/descending)
- Pagination (20 results per page, optional infinite scroll)
- File preview on row click (first 500 chars)
- Export button per result

**FR-UI-3:** Menu bar shall include:
- File → Exit
- Search → New Search, Clear History
- Tools → Settings
- Help → About, Documentation

### 3.2 Search Functionality
**FR-SEARCH-1:** Text search shall accept:
- Minimum 2 characters
- Maximum 5000 characters
- Unicode characters
- Special characters (normalized during comparison)

**FR-SEARCH-2:** Fuzzy matching algorithm shall:
- Normalize both query and file content (lowercase, leet speak conversion, punctuation removal)
- Calculate Levenshtein distance
- Return matches with similarity ≥ threshold
- Support typo tolerance (up to 2 character differences for 10-char strings)

**FR-SEARCH-3:** Search results shall be:
- Sorted by similarity score (highest first)
- Limited to 1000 results per query
- Returned within 5 seconds for datasets <5GB
- Cached for identical queries within 1 hour

### 3.3 Filtering & Configuration
**FR-FILTER-1:** File type filter shall support:
- Plain text (.txt)
- JSON (.json)
- XML (.xml)
- CSV (.csv)
- Markdown (.md)
- All types option

**FR-FILTER-2:** Date range filter shall:
- Accept start and end dates
- Default to "any date"
- Support presets: Last 24h, Last 7d, Last 30d, Custom

**FR-FILTER-3:** Threshold configuration shall:
- Allow values 0.0 to 1.0
- Default to 0.01
- Save user preference
- Show sensitivity hint (e.g., "0.01 = low precision, catches all variations")

### 3.4 Results & Export
**FR-EXPORT-1:** Export functionality shall:
- Export selected results to CSV
- Export selected results to JSON
- Include: filename, path, match score, size, modified date, query text
- Generate file with timestamp: `export_YYYY-MM-DD_HHMMSS.csv`

**FR-EXPORT-2:** Results shall display:
- Match percentage (0-100%)
- Full file path
- File size in human-readable format (KB, MB, GB)
- Modification date and time

---

## 4. Non-Functional Requirements

### 4.1 Performance
**NFR-PERF-1:** Response times
- Search initiation: <100ms
- File metadata scan: <1 second for 10,000 files
- Content matching: <5 seconds for 5GB dataset
- UI responsiveness: <100ms for user interactions
- Memory usage: <500MB for application

**NFR-PERF-2:** Scalability
- Support up to 100,000 files
- Support up to 50GB total dataset size
- Handle 10 concurrent search queries
- Cache up to 100 previous searches

### 4.2 Usability
**NFR-USE-1:** User interface shall be:
- Intuitive (new users productive within 5 minutes)
- Consistent with platform conventions (Windows/Linux/macOS)
- Accessible with keyboard navigation (Tab, Enter, Escape)
- Responsive to window resizing

**NFR-USE-2:** Help & documentation
- In-app help tooltips for all controls
- "?" help icon for search syntax
- About dialog with version info

### 4.3 Security
**NFR-SEC-1:** Access control
- Validate user has read permission on target directory
- Prevent path traversal attacks (reject ../, ../../)
- Log all search queries (optional, for audit)

**NFR-SEC-2:** Data protection
- Do not cache search results containing sensitive data
- Clear temporary cache on application exit
- Validate file ownership before access

### 4.4 Reliability
**NFR-REL-1:** Error handling
- Handle missing files gracefully
- Recover from permission denied errors
- Display user-friendly error messages
- Log detailed errors to file (~logs/error.log)

**NFR-REL-2:** Data integrity
- Validate search parameters before processing
- Verify file checksums on results display
- Warn if file modified during search

---

## 5. Technical Requirements

### 5.1 Architecture
**TR-ARCH-1:** Three-tier architecture:
- **UI Layer:** Qt GUI (C++)
- **Business Logic Layer:** Content fingerprinting engine (C++)
- **Data Access Layer:** File system scanner + indexer

**TR-ARCH-2:** Integration with existing compare.cpp
- Wrapper library to expose compare tool functions
- Plugin interface for custom matching algorithms

### 5.2 Platform Support
**TR-PLAT-1:** Supported operating systems:
- Linux (Ubuntu 20.04+)
- Windows 10/11
- macOS 10.15+

**TR-PLAT-2:** Minimum system requirements:
- CPU: Dual-core 2.0GHz
- RAM: 2GB
- Storage: 500MB for installation

### 5.3 Dependencies
- Qt 6.0+ (GUI framework)
- C++17 compiler (g++, MSVC, clang)
- OpenSSL (for hash functions)
- CMake 3.16+ (build system)

---

## 6. Use Cases

### UC-1: Basic Search
**Actor:** User  
**Precondition:** Dashboard open, protected files directory exists  
**Steps:**
1. User enters search text "password"
2. User clicks Search
3. System normalizes query
4. System scans protected files
5. System calculates similarity scores
6. System displays results sorted by match%

**Postcondition:** Results displayed with metadata

### UC-2: Filtered Search
**Actor:** User  
**Precondition:** Dashboard open  
**Steps:**
1. User enters search text
2. User selects file types: .txt, .json
3. User sets date range: Last 30 days
4. User sets threshold to 0.05
5. User clicks Search
6. System applies all filters before comparison

**Postcondition:** Only matching .txt and .json files from last 30 days displayed

### UC-3: Export Results
**Actor:** User  
**Precondition:** Search results displayed  
**Steps:**
1. User clicks Export button
2. User selects format (CSV/JSON)
3. System generates file
4. System saves to ~/Downloads/

**Postcondition:** File saved with timestamp

---

## 7. Acceptance Criteria

- [ ] Search returns correct matches with ≥95% accuracy
- [ ] UI responds to user input within 100ms
- [ ] All 1000 files searched within 5 seconds
- [ ] Threshold configuration saves between sessions
- [ ] Export files open correctly in Excel/notepad
- [ ] Application handles missing directories without crashing
- [ ] Path traversal attempts are blocked
- [ ] All keyboard shortcuts work
- [ ] Help documentation covers all features

---

## 8. Glossary

| Term | Definition |
|------|-----------|
| **Levenshtein Distance** | Minimum edits (insert, delete, substitute) to transform one string to another |
| **Fuzzy Matching** | Approximate string matching allowing typos and character variations |
| **Threshold** | Minimum similarity score (0-1) for a result to be included |
| **Normalize** | Transform text: lowercase, leet→letter, punctuation removal |
| **Fingerprinting** | Creating a normalized representation of content for comparison |

---

## 9. Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-03-31 | Initial SRS creation |

