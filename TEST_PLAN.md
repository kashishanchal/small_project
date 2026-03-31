# Test Plan & Testing Criteria
## GUI Dashboard for Content Fingerprinting & File Search

**Version:** 1.0  
**Date:** March 31, 2026  

---

## 1. Test Strategy Overview

### 1.1 Testing Levels
```
┌─────────────────────────────────────────┐
│  System Testing                         │
│  ├─ End-to-End scenarios               │
│  └─ Performance validation              │
├─────────────────────────────────────────┤
│  Integration Testing                    │
│  ├─ Component interactions             │
│  └─ Service-to-service                 │
├─────────────────────────────────────────┤
│  Unit Testing                           │
│  ├─ Individual functions               │
│  └─ Class methods                      │
├─────────────────────────────────────────┤
│  UI Testing                             │
│  ├─ User interactions                  │
│  └─ Visual validation                  │
└─────────────────────────────────────────┘
```

### 1.2 Test Execution Timeline
- **Phase 1:** Unit & Integration (Week 1-2)
- **Phase 2:** System & Performance (Week 2-3)
- **Phase 3:** UAT & Bug Fixes (Week 3-4)

---

## 2. Unit Test Suite

### 2.1 Comparator Module Tests

**Test Class:** `ComparatorServiceTest`

| ID | Test Case | Input | Expected Output | Criteria |
|----|-----------| ------|-----------------|----------|
| UT-COMP-001 | Exact match | ("hello", "hello") | similarity = 1.0 | ✓ Pass if score = 100% |
| UT-COMP-002 | No similarity | ("abc", "xyz") | similarity = ~0.0 | ✓ Pass if score < 10% |
| UT-COMP-003 | Partial match | ("hello world", "hello") | similarity ≥ 0.5 | ✓ Pass if score in range |
| UT-COMP-004 | Case insensitivity | ("HELLO", "hello") | similarity = 1.0 | ✓ Case normalized |
| UT-COMP-005 | Leet speak: "p@ssw0rd" | ("p@ssw0rd", "password") | similarity ≥ 0.9 | ✓ Leet converted |
| UT-COMP-006 | Special chars removed | ("hello!@#$", "hello") | similarity = 1.0 | ✓ Punctuation ignored |
| UT-COMP-007 | Multiple spaces | ("hello  world", "hello world") | similarity = 1.0 | ✓ Spaces normalized |
| UT-COMP-008 | Unicode handling | ("café", "cafe") | similarity ≥ 0.8 | ✓ Unicode handled |
| UT-COMP-009 | Empty string input | ("", "test") | similarity = 0.0 | ✓ Returns 0 |
| UT-COMP-010 | Very long strings | (5000 chars, 5000 chars) | similarity calculated | ✓ No timeout |

**Test Code Example:**
```cpp
TEST(ComparatorServiceTest, ExactMatch) {
    ComparatorService comp;
    double result = comp.getSimilarity("hello", "hello");
    EXPECT_EQ(result, 1.0);
}

TEST(ComparatorServiceTest, LeetSpeakNormalization) {
    ComparatorService comp;
    double result = comp.getSimilarity("p@ssw0rd", "password");
    EXPECT_GE(result, 0.9);
}
```

### 2.2 File Scanner Module Tests

**Test Class:** `FileScannerTest`

| ID | Test Case | Setup | Expected | Criteria |
|----|-----------|-------|----------|----------|
| UT-SCAN-001 | Scan valid directory | 5 .txt files | Returns 5 entries | ✓ All files found |
| UT-SCAN-002 | Scan empty directory | 0 files | Returns 0 entries | ✓ Handles empty |
| UT-SCAN-003 | Filter by extension | 10 files (5 .txt, 5 .json) | Returns 5 .txt files | ✓ Filter works |
| UT-SCAN-004 | Filter by date range | Files from 2026-01 to 2026-03 | Returns 60 files | ✓ Date filter works |
| UT-SCAN-005 | Handle missing directory | path = "/nonexistent/" | Throws exception | ✓ Exception caught |
| UT-SCAN-006 | Handle permission denied | Read-only directory | Throws exception | ✓ Permission error |
| UT-SCAN-007 | Retrieve file metadata | file.txt (1024 bytes) | Returns correct size, date | ✓ Metadata accurate |
| UT-SCAN-008 | Symlink handling | Symlink to ../secret | Resolves or rejects | ✓ Security validated |

### 2.3 Export Service Module Tests

| ID | Test Case | Input | Expected | Criteria |
|----|-----------|-------|----------|----------|
| UT-EXPORT-001 | CSV format | 3 results | CSV with headers | ✓ Valid CSV format |
| UT-EXPORT-002 | CSV escaping | "value with, comma" | Properly escaped value | ✓ CSV compliant |
| UT-EXPORT-003 | JSON format | 3 results | Valid JSON structure | ✓ Valid JSON |
| UT-EXPORT-004 | Empty results | 0 results | File with headers only | ✓ Handles empty |
| UT-EXPORT-005 | Special characters in path | "file (1).txt" | Unchanged in output | ✓ Path preserved |
| UT-EXPORT-006 | Large result set | 1000 results | File created successfully | ✓ No memory error |

---

## 3. Integration Test Suite

### 3.1 Search Workflow Integration Tests

**Test Scenario:** Complete search from input to results display

| ID | Test Case | Steps | Expected | Pass Criteria |
|----|-----------| ------|----------|---------------|
| IT-SEARCH-001 | Basic search flow | 1. Input "test" 2. Click search 3. Wait for results | Results displayed in table | ✓ UI updates, results shown |
| IT-SEARCH-002 | Search with file type filter | 1. Input "test" 2. Select .txt 3. Search | Only .txt files returned | ✓ Filter applied correctly |
| IT-SEARCH-003 | Search with date filter | 1. Input query 2. Set date range 3. Search | Results within date range | ✓ Date filter applied |
| IT-SEARCH-004 | Threshold configuration | 1. Set threshold to 0.5 2. Search | More results returned (lower threshold) | ✓ Threshold affects results |
| IT-SEARCH-005 | Cache hit | 1. Search "test" 2. Same search again | Second search returns instantly | ✓ Response < 50ms |
| IT-SEARCH-006 | Clear cache | 1. Search 2. Clear cache 3. Same search | Cache invalidated, results recalculated | ✓ Fresh calculation |

**Test Code Example:**
```cpp
TEST(SearchControllerIntegration, BasicSearchFlow) {
    SearchController controller;
    SearchQuery query;
    query.text = "confidential";
    query.threshold = 0.01;
    query.directoryPath = "./test_data";
    
    auto startTime = chrono::now();
    controller.executeSearch(query);
    auto results = controller.getResults();
    auto elapsed = chrono::now() - startTime;
    
    EXPECT_GT(results.size(), 0);
    EXPECT_LT(elapsed.count(), 5000);  // < 5 seconds
}
```

### 3.2 Service Integration Tests

| ID | Test Case | Services | Expected | Criteria |
|----|-----------|----------|----------|----------|
| IT-SERVICE-001 | FileScanner + ComparatorService | Scan files, then compare | Correct similarity scores | ✓ Integration works |
| IT-SERVICE-002 | SearchController + ExportService | Search, then export results | Valid export file created | ✓ Export contains results |
| IT-SERVICE-003 | Cache hit | Repeated queries hit cache | Cache returns identical results | ✓ Consistency maintained |

---

## 4. System Test Suite

### 4.1 End-to-End Scenarios

**Scenario E2E-001: User finds sensitive data**
```
Precondition: Protected files exist (5 files with "password")
Steps:
  1. User opens Dashboard
  2. User enters search text: "password"
  3. User sets threshold: 0.01
  4. User clicks Search
  5. User views results
  6. User clicks Export → CSV
  7. User opens exported file
  
Expected Result:
  ✓ Results show 5 matching files
  ✓ Similarity scores ≥ 90%
  ✓ CSV file created with correct data
  ✓ CSV opens in Excel correctly
```

**Scenario E2E-002: User filters by date and type**
```
Precondition: 100 files (50 .txt, 50 .json) from last 6 months
Steps:
  1. User opens Dashboard
  2. User selects file types: .txt only
  3. User sets date range: Last 30 days
  4. User enters search: "policy"
  5. User clicks Search
  
Expected Result:
  ✓ Only .txt files in results
  ✓ All files modified in last 30 days
  ✓ Results show only matching files
  ✓ Search completes within 2 seconds
```

### 4.2 Error Handling Scenarios

| ID | Scenario | Input | Expected Behavior |
|----|---------| ------|-------------------|
| E2E-ERR-001 | Directory not found | path = "/nonexistent/dir" | Error dialog shown, info logged |
| E2E-ERR-002 | Permission denied | Protected directory | Graceful error, suggestion to sudo |
| E2E-ERR-003 | Invalid query | Query = "" (empty) | Input validation error tooltip |
| E2E-ERR-004 | Invalid threshold | threshold = 1.5 (out of range) | Error message, value corrected to 1.0 |
| E2E-ERR-005 | Search timeout | Large dataset, complex query | Cancel option shown, partial results |
| E2E-ERR-006 | File disappeared | File deleted during search | Log warning, continue search |

---

## 5. Performance Test Suite

### 5.1 Response Time Tests

**Baseline Requirements:**
```
Search initialization: < 100ms
File scan (10,000 files): < 1s
Content matching (5GB): < 5s
Export (1000 results): < 500ms
UI interaction: < 100ms
Cache hit: < 50ms
```

**Test Cases:**

| ID | Dataset Size | Query Type | Expected Time | Pass if |
|----|--------------|-----------|----------------|---------|
| PERF-001 | 100 files (10MB) | Simple keyword | < 500ms | ✓ Time ≤ 500ms |
| PERF-002 | 1,000 files (100MB) | Simple keyword | < 1s | ✓ Time ≤ 1000ms |
| PERF-003 | 10,000 files (1GB) | Simple keyword | < 3s | ✓ Time ≤ 3000ms |
| PERF-004 | 100,000 files (5GB) | Simple keyword | < 10s | ✓ Time ≤ 10,000ms |
| PERF-005 | 1,000 files (100MB) | Complex fuzzy match | < 2s | ✓ Time ≤ 2000ms |
| PERF-006 | 1,000 files (100MB) | With all filters | < 1.5s | ✓ Time ≤ 1500ms |
| PERF-007 | Export 100 results | CSV format | < 100ms | ✓ Time ≤ 100ms |
| PERF-008 | Export 1000 results | JSON format | < 500ms | ✓ Time ≤ 500ms |

**Performance Test Code:**
```cpp
TEST(PerformanceTest, Search10KFiles) {
    SearchController controller;
    SearchQuery query;
    query.text = "test";
    query.directoryPath = "./test_data_10k";
    
    auto start = chrono::high_resolution_clock::now();
    controller.executeSearch(query);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<ms>(end - start);
    
    EXPECT_LE(duration.count(), 3000);  // < 3 seconds
}
```

### 5.2 Memory Usage Tests

| ID | Scenario | Max Memory | Pass if |
|----|----------|-----------|---------|
| MEM-001 | Application idle | < 50MB | ✓ Under 50MB |
| MEM-002 | Search 10K files | < 200MB | ✓ Under 200MB |
| MEM-003 | Cache 100 searches | < 300MB | ✓ Under 300MB |
| MEM-004 | Memory leak (10 searches) | No increase | ✓ Stable |

### 5.3 Scalability Tests

| ID | Test | Scale | Expected | Criteria |
|----|------|-------|----------|----------|
| SCALE-001 | Concurrent searches | 10 searches | No crashes | ✓ Threadsafe |
| SCALE-002 | Large dataset | 100,000 files | Graceful degradation | ✓ Completes in time |
| SCALE-003 | Large query result | 10,000 results | All displayed, no truncation | ✓ All results shown |

---

## 6. UI/UX Test Suite

### 6.1 Usability Tests

| ID | Test Case | Actions | Expected | Pass Criteria |
|----|-----------|---------|----------|---------------|
| UI-USE-001 | New user productivity | User opens app, performs search | Completes in < 5 min | ✓ Intuitive UI |
| UI-USE-002 | Keyboard navigation | Tab through fields, Enter to search | All controls accessible | ✓ Keyboard shortcuts work |
| UI-USE-003 | Window resize | Resize window to various sizes | Layout adapts correctly | ✓ Responsive design |
| UI-USE-004 | Error message clarity | Trigger error (no directory) | Error message understandable | ✓ Clear messaging |
| UI-USE-005 | Results table sorting | Click column headers | Sort ascending/descending | ✓ Sorting works |
| UI-USE-006 | Pagination | View results page 1, 2, 3 | Navigation works smoothly | ✓ Pagination complete |

### 6.2 Accessibility Tests

| ID | Requirement | Test | Pass Criteria |
|----|-------------|------|---------------|
| ACC-001 | Keyboard support | Navigate entire UI with Tab, Enter | ✓ All functions accessible |
| ACC-002 | Color contrast | Text meets WCAG AA standards | ✓ Readable colors |
| ACC-003 | Font size | Resizable at 120%, 150% | ✓ Readable at zoom |
| ACC-004 | Help text | All controls have tooltips | ✓ Help available |

---

## 7. Security Test Suite

### 7.1 Input Validation Tests

| ID | Test Case | Input | Expected | Pass Criteria |
|----|-----------|-------|----------|---------------|
| SEC-VAL-001 | Query length limit | Query > 5000 chars | Truncated/rejected | ✓ Validation works |
| SEC-VAL-002 | Path traversal attempt | path = "/../../../etc/passwd" | Rejected with error | ✓ Path blocked |
| SEC-VAL-003 | SQL injection (if DB) | query = "'; DROP TABLE--" | Sanitized/escaped | ✓ Query safe |
| SEC-VAL-004 | Invalid threshold | threshold = -1 or 2.5 | Corrected to valid range | ✓ Range enforced |
| SEC-VAL-005 | Null input | query = null | Handled gracefully | ✓ No crash |

### 7.2 Access Control Tests

| ID | Test Case | Condition | Expected | Pass Criteria |
|----|-----------|-----------|----------|---------------|
| SEC-ACC-001 | Permission denied | User lacks read permission | Error shown | ✓ Permission validated |
| SEC-ACC-002 | Symlink attack | Symlink to restricted dir | Rejected or resolved safely | ✓ Security preserved |
| SEC-ACC-003 | File disappears during search | File deleted mid-search | Skip file, log, continue | ✓ Resilience demonstrated |

---

## 8. Compatibility Test Suite

### 8.1 Platform Testing

| Platform | OS Version | Qt Version | Tested | Status |
|----------|-----------|-----------|--------|--------|
| Linux | Ubuntu 20.04 LTS | Qt 6.0 | ✓ | Target |
| Linux | Ubuntu 22.04 LTS | Qt 6.2 | ✓ | Target |
| Windows | Windows 10 | Qt 6.0 MSVC | ✓ | Target |
| Windows | Windows 11 | Qt 6.2 MSVC | ✓ | Target |
| macOS | Monterey (12) | Qt 6.0 | ✓ | Target |
| macOS | Ventura (13) | Qt 6.2 | ✓ | Target |

### 8.2 File Format Compatibility

| File Type | Test | Expected | Status |
|-----------|------|----------|--------|
| .txt | Read plain text | ✓ Reads correctly | ✓ Pass |
| .json | Parse JSON for content | ✓ Extracts text content | ✓ Pass |
| .xml | Parse XML for content | ✓ Extracts text content | ✓ Pass |
| .csv | Parse CSV for content | ✓ Extracts text content | ✓ Pass |
| .md | Read markdown | ✓ Reads correctly | ✓ Pass |
| .pdf | (Phase 2) | - | - |
| .docx | (Phase 2) | - | - |

---

## 9. Regression Test Suite

### 9.1 Critical Functions

After each code change, verify:

```
☐ Basic search still works
☐ Similarity scoring unchanged
☐ Export formatting correct
☐ Cache functionality intact
☐ No memory leaks introduced
☐ Performance baseline maintained
☐ UI still responsive
☐ Error handling robust
```

---

## 10. Test Execution & Reporting

### 10.1 Test Report Template

```
Test Execution Report
Date: 2026-04-01
Environment: Ubuntu 22.04 LTS, Qt 6.2
Build: Release (optimization enabled)

Test Summary:
├─ Unit Tests: 45/45 PASSED
├─ Integration Tests: 12/12 PASSED
├─ System Tests: 8/8 PASSED
├─ Performance Tests: 8/8 PASSED
├─ UI Tests: 6/6 PASSED
└─ Security Tests: 8/8 PASSED

Total: 87/87 PASSED (100%)

Coverage:
├─ Statement Coverage: 95%
├─ Branch Coverage: 92%
└─ Function Coverage: 100%

Performance:
├─ Average Search Time: 850ms (10K files)
├─ Memory Peak: 187MB
└─ Export Time: 85ms (1000 results)

Issues Found: 0 Critical, 0 High, 2 Medium
Resolution: Ready for Release
```

### 10.2 Bug Tracking

```
Bug Report Template
ID: BUG-001
Title: Search crashes with empty query
Severity: High
Reproduction: 1. Enter empty text 2. Click search
Expected: Error message shown
Actual: Application crashes
Status: OPEN → FIXED → VERIFIED
```

---

## 11. Test Environment Setup

### 11.1 Required Infrastructure

```bash
# Create test data directory
mkdir -p /tmp/test_data_dashboard

# Generate test files
for i in {1..100}; do
    echo "This is test file $i with password123" > /tmp/test_data/file_$i.txt
done

# Test protected directory
mkdir -p ~/Desktop/protected
echo "Confidential company policy document 2026" > ~/Desktop/protected/secret1.txt
```

### 11.2 CI/CD Integration

```yaml
# .github/workflows/test.yml
name: Test Dashboard
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt-get install -y qt6-base-dev cmake
      - name: Build
        run: cmake -B build && cmake --build build -j
      - name: Unit tests
        run: ./build/test_runner
      - name: Integration tests
        run: ./build/integration_test_runner
      - name: Coverage report
        run: gcov build/**/*.o && codecov
```

---

## 12. Acceptance Criteria Checklist

- [ ] 100% unit tests pass
- [ ] 95%+ code coverage achieved
- [ ] All performance baselines met
- [ ] No critical/high severity bugs remain
- [ ] All platforms tested and passed
- [ ] Security validation completed
- [ ] Documentation reviewed and updated
- [ ] UAT sign-off obtained
- [ ] Release notes prepared

