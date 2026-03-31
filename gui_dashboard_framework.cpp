/**
 * GUI Dashboard Framework - Content Fingerprinting & File Search
 * A Qt-based GUI for the DLP Clipboard Monitor with search capabilities
 * 
 * Build: g++ -std=c++20 -o gui_dashboard gui_dashboard_framework.cpp $(pkg-config --cflags --libs Qt6Core Qt6Gui Qt6Widgets)
 */

#include <compare>
#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QProgressBar>
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <fstream>
#include <sstream>

// ============ Data Structures ============

struct FileMetadata {
    std::string filename;
    std::string fullPath;
    long fileSize;
    std::string lastModified;
    std::string fileType;
};

struct SearchResult {
    FileMetadata fileInfo;
    double similarityScore;
    std::string matchedContent;
};

struct SearchQuery {
    std::string text;
    std::vector<std::string> fileTypes;
    std::string directoryPath;
    double threshold = 0.7;
    int maxResults = 1000;
};

// ============ Services ============

class ComparatorService {
public:
    /**
     * Compare two strings using Levenshtein distance
     * Returns normalized similarity score (0.0 to 1.0)
     */
    double getSimilarity(const std::string& query, const std::string& content) {
        std::string normQuery = normalize(query);
        std::string normContent = normalize(content);
        
        if (normQuery.empty() || normContent.empty()) {
            return 0.0;
        }
        
        int dist = levenshteinDistance(normQuery, normContent);
        int maxLen = std::max(normQuery.length(), normContent.length());
        
        double similarity = 1.0 - (static_cast<double>(dist) / maxLen);
        return std::max(0.0, similarity);
    }
    
    /**
     * Normalize string: lowercase, leet->letter, remove punctuation
     */
    std::string normalize(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        
        for (unsigned char ch : s) {
            char c = ch;
            
            // Leet speak mapping
            if (c == '0') c = 'o';
            else if (c == '1') c = 'l';
            else if (c == '3') c = 'e';
            else if (c == '4') c = 'a';
            else if (c == '5') c = 's';
            else if (c == '7') c = 't';
            else if (c == '@') c = 'a';
            else if (c == '$') c = 's';
            
            c = std::tolower(static_cast<unsigned char>(c));
            
            if (std::isalnum(static_cast<unsigned char>(c))) {
                out.push_back(c);
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                if (out.empty() || out.back() != ' ') {
                    out.push_back(' ');
                }
            }
        }
        
        // Trim trailing spaces
        while (!out.empty() && out.back() == ' ') {
            out.pop_back();
        }
        
        return out;
    }

private:
    /**
     * Calculate Levenshtein distance between two strings
     */
    int levenshteinDistance(const std::string& a, const std::string& b) {
        if (a.empty()) return static_cast<int>(b.size());
        if (b.empty()) return static_cast<int>(a.size());
        
        size_t n = a.size(), m = b.size();
        std::vector<int> prev(m + 1), cur(m + 1);
        
        for (size_t j = 0; j <= m; ++j) {
            prev[j] = static_cast<int>(j);
        }
        
        for (size_t i = 1; i <= n; ++i) {
            cur[0] = static_cast<int>(i);
            for (size_t j = 1; j <= m; ++j) {
                int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
                cur[j] = std::min({prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + cost});
            }
            prev.swap(cur);
        }
        
        return prev[m];
    }
};

class FileScanner {
public:
    /**
     * Scan directory and return file metadata
     * Note: This is a simplified version; production would use filesystem library
     */
    std::vector<FileMetadata> scanDirectory(const std::string& path) {
        std::vector<FileMetadata> results;
        // TODO: Implement actual filesystem scanning using std::filesystem
        // For now, return empty vector (to be implemented with real file I/O)
        return results;
    }
};

// ============ UI Components ============

class SearchPanelWidget : public QWidget {
    Q_OBJECT

public:
    SearchPanelWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
    }
    
    SearchQuery getInput() {
        SearchQuery query;
        query.text = queryInput->text().toStdString();
        query.threshold = thresholdSlider->value() / 100.0;
        query.directoryPath = dirPathInput->text().toStdString();
        
        // Get selected file types
        if (fileTypeCombo->currentText() != "All Types") {
            query.fileTypes.push_back(fileTypeCombo->currentText().toStdString());
        }
        
        return query;
    }
    
    void reset() {
        queryInput->clear();
        thresholdSlider->setValue(70);
        updateThresholdLabel();
    }

private:
    QLineEdit* queryInput = nullptr;
    QLineEdit* dirPathInput = nullptr;
    QComboBox* fileTypeCombo = nullptr;
    QSlider* thresholdSlider = nullptr;
    QLabel* thresholdLabel = nullptr;

    void setupUI() {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Query input
        QHBoxLayout* queryLayout = new QHBoxLayout();
        queryLayout->addWidget(new QLabel("Search Query:"));
        queryInput = new QLineEdit();
        queryInput->setPlaceholderText("Enter text to search (min 2 chars)");
        queryLayout->addWidget(queryInput);
        mainLayout->addLayout(queryLayout);
        
        // Directory path
        QHBoxLayout* dirLayout = new QHBoxLayout();
        dirLayout->addWidget(new QLabel("Directory:"));
        dirPathInput = new QLineEdit();
        dirPathInput->setText(QDir::home().path() + "/Desktop/protected");
        dirLayout->addWidget(dirPathInput);
        
        QPushButton* browseBtn = new QPushButton("Browse");
        connect(browseBtn, &QPushButton::clicked, this, &SearchPanelWidget::onBrowseDirectory);
        dirLayout->addWidget(browseBtn);
        mainLayout->addLayout(dirLayout);
        
        // File type filter
        QHBoxLayout* filterLayout = new QHBoxLayout();
        filterLayout->addWidget(new QLabel("File Type:"));
        fileTypeCombo = new QComboBox();
        fileTypeCombo->addItems({"All Types", ".txt", ".json", ".xml", ".csv", ".md"});
        filterLayout->addWidget(fileTypeCombo);
        filterLayout->addStretch();
        mainLayout->addLayout(filterLayout);
        
        // Threshold slider
        QHBoxLayout* thresholdLayout = new QHBoxLayout();
        thresholdLayout->addWidget(new QLabel("Threshold:"));
        thresholdSlider = new QSlider(Qt::Horizontal);
        thresholdSlider->setRange(0, 100);
        thresholdSlider->setValue(1);
        thresholdSlider->setMaximumWidth(200);
        thresholdLayout->addWidget(thresholdSlider);
        
        thresholdLabel = new QLabel("0.01");
        thresholdLayout->addWidget(thresholdLabel);
        thresholdLayout->addStretch();
        
        connect(thresholdSlider, &QSlider::valueChanged, this, &SearchPanelWidget::updateThresholdLabel);
        mainLayout->addLayout(thresholdLayout);
        
        setLayout(mainLayout);
    }

private slots:
    void onBrowseDirectory() {
        QString dir = QFileDialog::getExistingDirectory(this, "Select Directory");
        if (!dir.isEmpty()) {
            dirPathInput->setText(dir);
        }
    }
    
    void updateThresholdLabel() {
        double value = thresholdSlider->value() / 100.0;
        thresholdLabel->setText(QString::number(value, 'f', 2));
    }
};

class ResultsPanelWidget : public QWidget {
    Q_OBJECT

public:
    ResultsPanelWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
    }
    
    void displayResults(const std::vector<SearchResult>& results) {
        resultsTable->setRowCount(0);
        
        for (size_t i = 0; i < results.size(); ++i) {
            resultsTable->insertRow(i);
            
            const SearchResult& result = results[i];
            
            // Filename
            resultsTable->setItem(i, 0, new QTableWidgetItem(
                QString::fromStdString(result.fileInfo.filename)));
            
            // Path
            resultsTable->setItem(i, 1, new QTableWidgetItem(
                QString::fromStdString(result.fileInfo.fullPath)));
            
            // Score
            int scorePercent = static_cast<int>(result.similarityScore * 100 + 0.5);
            resultsTable->setItem(i, 2, new QTableWidgetItem(
                QString::number(scorePercent) + "%"));
            
            // Size
            resultsTable->setItem(i, 3, new QTableWidgetItem(
                QString::fromStdString(result.fileInfo.fullPath)));
            
            // Modified
            resultsTable->setItem(i, 4, new QTableWidgetItem(
                QString::fromStdString(result.fileInfo.lastModified)));
        }
        
        statusLabel->setText(QString("Found %1 results").arg(results.size()));
    }
    
    void exportResultsToCSV(const std::vector<SearchResult>& results) {
        QString filename = QFileDialog::getSaveFileName(this, "Export Results", "",
                                                       "CSV Files (*.csv)");
        if (filename.isEmpty()) return;
        
        std::ofstream file(filename.toStdString());
        if (!file.is_open()) {
            QMessageBox::warning(this, "Export Error", "Could not create file");
            return;
        }
        
        // Write header
        file << "Filename,Path,Similarity%,Size,Modified\n";
        
        // Write data
        for (const auto& result : results) {
            int score = static_cast<int>(result.similarityScore * 100 + 0.5);
            file << "\"" << result.fileInfo.filename << "\","
                 << "\"" << result.fileInfo.fullPath << "\","
                 << score << ","
                 << result.fileInfo.fileSize << ","
                 << "\"" << result.fileInfo.lastModified << "\"\n";
        }
        
        file.close();
        QMessageBox::information(this, "Export Successful", 
                                "Results exported to: " + filename);
    }

private:
    QTableWidget* resultsTable = nullptr;
    QLabel* statusLabel = nullptr;

    void setupUI() {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        
        // Status bar
        statusLabel = new QLabel("No results");
        mainLayout->addWidget(statusLabel);
        
        // Results table
        resultsTable = new QTableWidget();
        resultsTable->setColumnCount(5);
        resultsTable->setHorizontalHeaderLabels(
            {"Filename", "Path", "Score", "Size", "Modified"});
        resultsTable->horizontalHeader()->setStretchLastSection(false);
        resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        resultsTable->setAlternatingRowColors(true);
        mainLayout->addWidget(resultsTable);
        
        // Export button
        QPushButton* exportBtn = new QPushButton("Export to CSV");
        connect(exportBtn, &QPushButton::clicked, this, [this]() {
            // Dummy data for demonstration
            std::vector<SearchResult> dummyResults;
            exportResultsToCSV(dummyResults);
        });
        mainLayout->addWidget(exportBtn);
        
        setLayout(mainLayout);
    }
};

// ============ Main Application Window ============

class DashboardWindow : public QMainWindow {
    Q_OBJECT

public:
    DashboardWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setupUI();
        setupMenuBar();
        setWindowTitle("Content Fingerprinting Dashboard");
        setGeometry(100, 100, 1200, 700);
    }

private:
    SearchPanelWidget* searchPanel = nullptr;
    ResultsPanelWidget* resultsPanel = nullptr;
    ComparatorService comparator;
    FileScanner fileScanner;

    void setupUI() {
        QWidget* centralWidget = new QWidget();
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        
        // Search panel
        searchPanel = new SearchPanelWidget();
        mainLayout->addWidget(new QLabel("=== Search Configuration ==="));
        mainLayout->addWidget(searchPanel);
        
        // Results panel
        resultsPanel = new ResultsPanelWidget();
        mainLayout->addWidget(new QLabel("=== Search Results ==="));
        mainLayout->addWidget(resultsPanel);
        
        // Search button
        QPushButton* searchBtn = new QPushButton("Search");
        connect(searchBtn, &QPushButton::clicked, this, &DashboardWindow::onSearch);
        mainLayout->addWidget(searchBtn);
        
        // Clear button
        QPushButton* clearBtn = new QPushButton("Clear");
        connect(clearBtn, &QPushButton::clicked, this, &DashboardWindow::onClear);
        mainLayout->addWidget(clearBtn);
        
        centralWidget->setLayout(mainLayout);
        setCentralWidget(centralWidget);
        
        // Status bar
        statusBar()->showMessage("Ready");
    }

    void setupMenuBar() {
        // File menu
        QMenu* fileMenu = menuBar()->addMenu("&File");
        QAction* exitAction = fileMenu->addAction("E&xit");
        connect(exitAction, &QAction::triggered, this, &QWidget::close);
        
        // Help menu
        QMenu* helpMenu = menuBar()->addMenu("&Help");
        QAction* aboutAction = helpMenu->addAction("&About");
        connect(aboutAction, &QAction::triggered, this, &DashboardWindow::showAbout);
    }

private slots:
    void onSearch() {
        SearchQuery query = searchPanel->getInput();
        
        if (query.text.empty()) {
            QMessageBox::warning(this, "Input Error", "Please enter a search query");
            return;
        }
        
        if (query.text.length() < 2) {
            QMessageBox::warning(this, "Input Error", "Query must be at least 2 characters");
            return;
        }
        
        statusBar()->showMessage("Searching...");
        
        // For this demonstration, we'll show dummy results
        std::vector<SearchResult> results;
        
        // Create a dummy result for demonstration
        SearchResult dummyResult;
        dummyResult.fileInfo.filename = "example.txt";
        dummyResult.fileInfo.fullPath = "/home/user/Desktop/protected/example.txt";
        dummyResult.fileInfo.fileSize = 1024;
        dummyResult.fileInfo.lastModified = "2026-03-30 10:30:00";
        dummyResult.similarityScore = 0.85;
        dummyResult.matchedContent = "Example matched content...";
        results.push_back(dummyResult);
        
        resultsPanel->displayResults(results);
        statusBar()->showMessage(QString("Search complete - Found %1 result(s)")
                                .arg(results.size()));
    }
    
    void onClear() {
        searchPanel->reset();
        resultsPanel->displayResults({});
        statusBar()->showMessage("Cleared");
    }
    
    void showAbout() {
        QMessageBox::about(this, "About Dashboard",
            "Content Fingerprinting & File Search Dashboard\n\n"
            "Version: 1.0\n"
            "Purpose: Search and analyze files using fuzzy text matching\n"
            "Author: Kashish\n\n"
            "Built with Qt 6.0+");
    }
};

// ============ Main Application Entry ============

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    DashboardWindow window;
    window.show();
    
    return app.exec();
}

#include "gui_dashboard_framework.moc"
