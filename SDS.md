# Software Design Specification (SDS)
## GUI Dashboard for Content Fingerprinting & File Search

**Version:** 1.0  
**Date:** March 31, 2026  
**Project:** DLP Clipboard Monitor - GUI Dashboard Enhancement  

---

## 1. Design Overview

### 1.1 Architecture Pattern
```
┌─────────────────────────────────────────────────────┐
│              GUI Layer (Qt)                          │
│  ┌──────────────────────────────────────────────┐   │
│  │  Main Window                                  │   │
│  │  ├─ MenuBar (File, Search, Tools, Help)     │   │
│  │  ├─ SearchPanel (Input + Filters)           │   │
│  │  └─ ResultsPanel (Table + Export)           │   │
│  └──────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────┤
│         Controller/Business Logic Layer              │
│  ┌──────────────────────────────────────────────┐   │
│  │ SearchController                              │   │
│  │ ├─ validateInput()                            │   │
│  │ ├─ buildSearchQuery()                         │   │
│  │ └─ executeSearch()                            │   │
│  └──────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────┤
│         Service Layer                               │
│  ┌──────────────┬──────────────┬─────────────────┐  │
│  │ FileScanner  │ IndexService │ ComparatorSvc   │  │
│  │              │              │                 │  │
│  │ • listFiles()│ • buildIdx() │ • compare()    │  │
│  │ • getMetadata│ • queryIdx() │ • normalize()  │  │
│  └──────────────┴──────────────┴─────────────────┘  │
├─────────────────────────────────────────────────────┤
│         Data Layer                                  │
│  ┌──────────────┬──────────────┬─────────────────┐  │
│  │ FileSystem   │ Cache        │ compare.cpp     │  │
│  │ Scanner      │ (Search Idx) │ (Comparator)    │  │
│  └──────────────┴──────────────┴─────────────────┘  │
└─────────────────────────────────────────────────────┘
```

### 1.2 Design Principles
- **Separation of Concerns:** GUI, Business Logic, Data layers independent
- **Reusability:** Comparator as external library/wrapper
- **Caching:** Index and search results cached for performance
- **Extensibility:** Plugin interface for custom algorithms

---

## 2. System Architecture

### 2.1 High-Level Component Diagram
```
main.cpp
  └─ MainWindow (Qt Widget)
       ├─ MenuBar
       ├─ SearchPanelWidget
       │   ├─ QLineEdit (query)
       │   ├─ QComboBox (file types)
       │   ├─ QDateEdit (date range)
       │   ├─ QSlider (threshold)
       │   └─ QPushButton (search)
       ├─ ResultsPanelWidget
       │   ├─ QTableWidget (results)
       │   ├─ QPushButton (export)
       │   └─ QLabel (status)
       └─ SearchController
           ├─ FileScanner
           ├─ IndexService
           ├─ ComparatorService
           └─ ExportService
```

### 2.2 Class Diagram (Core Components)

```cpp
// Main Application Entry
class Dashboard {
public:
    void run();
private:
    MainWindow* window;
    SearchController* controller;
};

// UI Components
class MainWindow : public QMainWindow {
private:
    SearchPanelWidget* searchPanel;
    ResultsPanelWidget* resultsPanel;
    void setupMenuBar();
    void connectSignals();
};

class SearchPanelWidget : public QWidget {
public:
    SearchQuery getInput();
    void reset();
private:
    QLineEdit* queryInput;
    QComboBox* fileTypeFilter;
    QDateEdit* startDate, endDate;
    QSlider* thresholdSlider;
    QPushButton* searchButton;
};

class ResultsPanelWidget : public QWidget {
public:
    void displayResults(const vector<SearchResult>& results);
    void exportResults(ExportFormat format);
private:
    QTableWidget* resultsTable;
    void setupTableColumns();
    void populateTable(const vector<SearchResult>& results);
};

// Business Logic Layer
class SearchController : public QObject {
public:
    void executeSearch(const SearchQuery& query);
    vector<SearchResult> getResults();
private:
    FileScanner* fileScanner;
    IndexService* indexService;
    ComparatorService* comparator;
    void validateQuery(const SearchQuery& query);
    void filterResults(vector<SearchResult>& results);
};

// Services
class FileScanner {
public:
    vector<FileMetadata> scanDirectory(const string& path);
    vector<FileMetadata> filterByType(const vector<FileMetadata>& files,
                                       const vector<string>& types);
    vector<FileMetadata> filterByDateRange(const vector<FileMetadata>& files,
                                           time_t start, time_t end);
private:
    vector<FileMetadata> results;
};

class ComparatorService {
public:
    double calculateSimilarity(const string& query, const string& fileContent);
    string normalize(const string& text);
private:
    // Wrapper to compare.cpp functions
    int levenshteinDistance(const string& a, const string& b);
};

class ExportService {
public:
    void exportToCSV(const vector<SearchResult>& results, const string& filename);
    void exportToJSON(const vector<SearchResult>& results, const string& filename);
private:
    string escapeCSV(const string& field);
};

// Data Models
struct SearchQuery {
    string text;
    vector<string> fileTypes;  // ["txt", "json", "xml"]
    time_t dateStart, dateEnd;
    string directoryPath;
    double threshold = 0.7;
    int maxResults = 1000;
};

struct FileMetadata {
    string filename;
    string fullPath;
    long fileSize;
    time_t lastModified;
    string fileType;
};

struct SearchResult {
    FileMetadata fileInfo;
    double similarityScore;
    string matchedContent;  // Preview (first 500 chars)
};

struct Cache {
    map<string, vector<SearchResult>> searchCache;  // keyed by normalized query
    map<string, vector<FileMetadata>> indexCache;    // keyed by directory path
    time_t cacheExpiry = 3600;  // 1 hour
};
```

---

## 3. Data Flow Diagram

### 3.1 Search Flow
```
User Input
  │
  ├─ Validate Query (length, type)
  │
  ├─ Check Cache (if exists and valid)
  │  └─ Return cached results ─→ Display
  │
  ├─ Check Index Cache
  │  └─ Use cached file index
  │  OR
  │  └─ Scan directory (FileScanner)
  │
  ├─ Apply Filters
  │  ├─ File type (.txt, .json, etc.)
  │  └─ Date range
  │
  ├─ Compare Content
  │  ├─ Normalize query & file content
  │  ├─ Calculate Levenshtein distance
  │  └─ Filter by threshold
  │
  ├─ Sort by similarity (descending)
  │
  ├─ Cache results
  │
  └─ Display Results (Table + Count)
```

### 3.2 Export Flow
```
User clicks Export
  │
  ├─ Select format (CSV or JSON)
  │
  ├─ Validate selected results
  │
  ├─ Format output
  │  ├─ CSV: filename,path,score,size,modified
  │  └─ JSON: { "results": [...], "query": "...", "timestamp": "..." }
  │
  ├─ Generate filename (export_YYYY-MM-DD_HHMMSS.ext)
  │
  └─ Save to ~/Downloads/
```

---

## 4. Module Specifications

### 4.1 GUI Module (Qt-based)

**File:** `src/gui/main_window.cpp`

**Responsibilities:**
- Render main application window
- Handle user interactions (button clicks, text input)
- Emit signals to controller
- Display results and status messages

**Key Classes:**
- `MainWindow` - Main application window
- `SearchPanelWidget` - Search input controls
- `ResultsPanelWidget` - Results display table
- `SettingsDialog` - Settings/preferences

**Signals & Slots:**
```cpp
signals:
    void searchRequested(SearchQuery query);
    void exportRequested(ExportFormat format);
    void settingsChanged(Settings newSettings);

slots:
    void onSearchResults(vector<SearchResult> results);
    void onSearchError(string errorMsg);
    void onSearchProgress(int fileCount, int processed);
```

### 4.2 Controller Module

**File:** `src/controller/search_controller.cpp`

**Responsibilities:**
- Coordinate between UI and services
- Validate input
- Orchestrate search workflow
- Manage caching

**Key Methods:**
```cpp
class SearchController {
    void executeSearch(SearchQuery query);
    void cancelSearch();
    void clearCache();
    vector<SearchResult> getCachedResults(string queryHash);
    void setCacheExpiry(int seconds);
};
```

### 4.3 Service Layer

**FileScanner Service:**
```cpp
class FileScanner {
    vector<FileMetadata> scanDirectory(string path);
    vector<FileMetadata> filterByExtension(vector<FileMetadata> files,
                                           vector<string> extensions);
    vector<FileMetadata> filterByDateRange(vector<FileMetadata> files,
                                           time_t start, time_t end);
};
```

**ComparatorService (wrapper to compare.cpp):**
```cpp
class ComparatorService {
    double getSimilarity(string query, string fileContent);
    string normalize(string text);
    int getLevenshteinDistance(string a, string b);
};
```

**ExportService:**
```cpp
class ExportService {
    void toCSV(vector<SearchResult> results, string outputPath);
    void toJSON(vector<SearchResult> results, string outputPath);
    string generateFilename(ExportFormat format);
};
```

### 4.4 Data Layer

**Cache Manager:**
```cpp
class CacheManager {
    void set(string key, vector<SearchResult> results);
    vector<SearchResult>* get(string key);
    bool isValid(string key);
    void clear();
    void expireOldEntries();
};
```

---

## 5. Algorithm Specifications

### 5.1 Search Algorithm (3-Tier)

**Tier 1: Metadata Matching (O(n))**
```
For each file in directory:
    if file_type in selected_types:
        if modified_date in date_range:
            include in candidates
```

**Tier 2: Index Lookup (O(log n))**
```
For each candidate file:
    create inverted index of tokens
    if query tokens in index:
        mark as "high priority"
```

**Tier 3: Content Similarity (O(m*n))**
```
For each candidate file:
    content = read_file(file_path)
    normalized_query = normalize(query)
    normalized_content = normalize(content)
    distance = levenshtein_distance(norm_query, norm_content)
    similarity = 1.0 - (distance / max_length)
    if similarity >= threshold:
        include in results
sort results by similarity (descending)
```

**Complexity Analysis:**
- Worst case: O(n*m) where n=files, m=avg file length
- Best case (with cache): O(1)
- Average case: O(n + k*m) where k=filtered candidates

### 5.2 Levenshtein Distance (from compare.cpp)

```
Pseudocode:
function levenshtein(a, b):
    create matrix[len(a)+1][len(b)+1]
    for i = 0 to len(a):
        matrix[i][0] = i
    for j = 0 to len(b):
        matrix[0][j] = j
    
    for i = 1 to len(a):
        for j = 1 to len(b):
            cost = 0 if a[i-1] == b[j-1] else 1
            matrix[i][j] = min(
                matrix[i-1][j] + 1,      // deletion
                matrix[i][j-1] + 1,      // insertion
                matrix[i-1][j-1] + cost  // substitution
            )
    
    return matrix[len(a)][len(b)]
```

---

## 6. Database Design (Cache Schema)

```sql
-- Search Cache Table
CREATE TABLE search_cache (
    cache_id INTEGER PRIMARY KEY,
    query_hash VARCHAR(64) UNIQUE,
    query_text VARCHAR(5000),
    results_count INT,
    results_json TEXT,
    created_at TIMESTAMP,
    expires_at TIMESTAMP
);

-- File Index Cache
CREATE TABLE file_index_cache (
    index_id INTEGER PRIMARY KEY,
    directory_path VARCHAR(4096),
    file_count INT,
    index_json TEXT,
    created_at TIMESTAMP,
    expires_at TIMESTAMP
);

-- Search History
CREATE TABLE search_history (
    history_id INTEGER PRIMARY KEY,
    query_text VARCHAR(5000),
    file_types VARCHAR(256),
    threshold FLOAT,
    result_count INT,
    search_time_ms INT,
    timestamp TIMESTAMP
);
```

---

## 7. Interface Specifications

### 7.1 REST API (if backend service needed)

```
POST /api/search
{
    "query": "string",
    "fileTypes": ["txt", "json"],
    "dateStart": "2026-01-01",
    "dateEnd": "2026-03-31",
    "threshold": 0.01,
    "maxResults": 1000
}

Response:
{
    "success": true,
    "results": [
        {
            "filename": "file1.txt",
            "path": "/home/user/Desktop/protected/file1.txt",
            "similarity": 0.85,
            "fileSize": 1024,
            "lastModified": "2026-03-30T14:30:00Z",
            "preview": "first 500 characters..."
        }
    ],
    "searchTime_ms": 150,
    "totalResults": 150
}
```

### 7.2 Configuration File Format

**File:** `~/.dashboard/config.json`
```json
{
    "theme": "light",
    "defaultDirectory": "~/Desktop/protected",
    "defaultThreshold": 0.01,
    "cacheExpiry_seconds": 3600,
    "maxResults": 1000,
    "supportedFileTypes": ["txt", "json", "xml", "csv", "md"],
    "recentSearches": 10
}
```

---

## 8. Security Considerations

### 8.1 Input Validation
- Query length: 2-5000 characters
- Directory path: reject "../", "../../"
- File types: whitelist only safe extensions
- Threshold: 0.0 ≤ threshold ≤ 1.0

### 8.2 File Access
- Check file permissions before reading
- Validate symlink targets (no escape)
- Error on permission denied (don't expose path)

### 8.3 Cache Security
- Don't cache sensitive query content
- Clear cache on sensitive operations
- Encrypt cache at rest (optional)

---

## 9. Error Handling

**Error Categories:**

| Error | Code | Action |
|-------|------|--------|
| Directory not found | E001 | Show dialog, suggest default |
| Permission denied | E002 | Log, show user-friendly msg |
| Invalid query | E003 | Highlight input field, tooltip |
| File disappeared | E004 | Skip file, log, continue |
| Search timeout | E005 | Cancel, show results so far |
| Export failed | E006 | Show save dialog, retry |

---

## 10. Performance Optimization

### 10.1 Caching Strategy
- Cache index of files in directory (1 hour TTL)
- Cache search results by query hash (1 hour TTL)
- Limit cache size to 100 entries

### 10.2 Parallelization
- Multi-threaded file scanning (4 threads)
- Parallel similarity calculation per file
- Non-blocking UI using Qt QThread

### 10.3 Indexing
- Build inverted index on app startup
- Lazy loading for large datasets
- Incremental indexing if files modified

---

## 11. Testing Strategy

### 11.1 Unit Tests
- Test normalize() function with edge cases
- Test levenshtein_distance calculation
- Test filter functions (type, date)
- Test export formatting (CSV, JSON)

### 11.2 Integration Tests
- End-to-end search workflow
- FileScanner + Comparator integration
- Export functionality with real files

### 11.3 UI Tests (Qt)
- Button interactions
- Input validation
- Result table updates
- Window resizing

---

## 12. Deployment

### 12.1 Build System
```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -j
```

### 12.2 Installation
```bash
cmake --install build --prefix /usr/local
# Creates: /usr/local/bin/dashboard
```

### 12.3 Configuration
```bash
mkdir -p ~/.dashboard
cp config.json ~/.dashboard/
```

