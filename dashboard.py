#!/usr/bin/env python3
"""
GUI Dashboard for Content Fingerprinting & File Search
Uses PyQt5 for the GUI and wraps the C++ comparator tool
"""

import sys
import os
import subprocess
import json
from pathlib import Path
from datetime import datetime
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QLineEdit, QPushButton, QTableWidget, QTableWidgetItem,
    QComboBox, QSlider, QFileDialog, QMessageBox, QMenuBar, QMenu,
    QSpinBox, QDoubleSpinBox, QDateEdit, QProgressBar
)
from PyQt5.QtCore import Qt, QDate, pyqtSignal, QThread
from PyQt5.QtGui import QIcon, QFont

class SearchWorker(QThread):
    """Worker thread for background search"""
    finished = pyqtSignal(list)
    error = pyqtSignal(str)
    progress = pyqtSignal(int, int)

    def __init__(self, query, protected_dir, threshold, file_types):
        super().__init__()
        self.query = query
        self.protected_dir = protected_dir
        self.threshold = threshold
        self.file_types = file_types

    def run(self):
        try:
            # Get the directory of the C++ compare tool
            compare_tool = Path(__file__).parent / "compare"
            
            if not compare_tool.exists():
                self.error.emit(f"Comparator tool not found: {compare_tool}")
                return

            # Build command
            cmd = [str(compare_tool)]
            cmd.extend(["--protected-dir", self.protected_dir])
            cmd.extend(["--threshold", str(self.threshold)])
            cmd.extend(["--input", self.query])

            # Run comparator
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            
            if result.returncode not in [0, 1]:
                self.error.emit(f"Error: {result.stderr}")
                return

            # Parse output (simplified for first match)
            results = []
            for line in result.stdout.strip().split('\n'):
                if line:
                    parts = line.split()
                    if len(parts) >= 3:
                        status = parts[0]  # MATCH or NO_MATCH
                        score = int(parts[1]) if parts[1].isdigit() else 0
                        path = ' '.join(parts[2:])
                        
                        results.append({
                            'filename': Path(path).name,
                            'path': path,
                            'score': score,
                            'status': status
                        })

            self.finished.emit(results)

        except subprocess.TimeoutExpired:
            self.error.emit("Search timed out (>30 seconds)")
        except Exception as e:
            self.error.emit(str(e))

class SearchPanelWidget(QWidget):
    """Search configuration panel"""
    
    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout()

        # Query input
        query_layout = QHBoxLayout()
        query_layout.addWidget(QLabel("Search Query:"))
        self.query_input = QLineEdit()
        self.query_input.setPlaceholderText("Enter text to search (min 2 chars)")
        query_layout.addWidget(self.query_input)
        layout.addLayout(query_layout)

        # Directory path
        dir_layout = QHBoxLayout()
        dir_layout.addWidget(QLabel("Directory:"))
        self.dir_input = QLineEdit()
        home = str(Path.home())
        self.dir_input.setText(f"{home}/Desktop/protected")
        dir_layout.addWidget(self.dir_input)
        
        browse_btn = QPushButton("Browse")
        browse_btn.clicked.connect(self.browse_directory)
        dir_layout.addWidget(browse_btn)
        layout.addLayout(dir_layout)

        # File type filter
        filter_layout = QHBoxLayout()
        filter_layout.addWidget(QLabel("File Type:"))
        self.file_type_combo = QComboBox()
        self.file_type_combo.addItems(["All Types", ".txt", ".json", ".xml", ".csv", ".md"])
        filter_layout.addWidget(self.file_type_combo)
        filter_layout.addStretch()
        layout.addLayout(filter_layout)

        # Threshold slider
        threshold_layout = QHBoxLayout()
        threshold_layout.addWidget(QLabel("Threshold:"))
        self.threshold_slider = QSlider(Qt.Horizontal)
        self.threshold_slider.setRange(0, 100)
        self.threshold_slider.setValue(1)
        self.threshold_slider.setMaximumWidth(200)
        self.threshold_slider.valueChanged.connect(self.update_threshold_label)
        threshold_layout.addWidget(self.threshold_slider)
        
        self.threshold_label = QLabel("0.01")
        threshold_layout.addWidget(self.threshold_label)
        threshold_layout.addStretch()
        layout.addLayout(threshold_layout)

        self.setLayout(layout)

    def browse_directory(self):
        directory = QFileDialog.getExistingDirectory(self, "Select Directory")
        if directory:
            self.dir_input.setText(directory)

    def update_threshold_label(self):
        value = self.threshold_slider.value() / 100.0
        self.threshold_label.setText(f"{value:.2f}")

    def get_threshold(self):
        return self.threshold_slider.value() / 100.0

    def get_query(self):
        return self.query_input.text()

    def get_directory(self):
        return self.dir_input.text()

    def get_file_type(self):
        return self.file_type_combo.currentText()

    def reset(self):
        self.query_input.clear()
        self.threshold_slider.setValue(1)

class ResultsPanelWidget(QWidget):
    """Results display panel"""
    
    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        layout = QVBoxLayout()

        # Status label
        self.status_label = QLabel("No results")
        layout.addWidget(self.status_label)

        # Progress bar
        self.progress_bar = QProgressBar()
        self.progress_bar.setVisible(False)
        layout.addWidget(self.progress_bar)

        # Results table
        self.results_table = QTableWidget()
        self.results_table.setColumnCount(4)
        self.results_table.setHorizontalHeaderLabels(["Filename", "Path", "Score", "Status"])
        self.results_table.resizeColumnsToContents()
        layout.addWidget(self.results_table)

        # Export button
        button_layout = QHBoxLayout()
        export_btn = QPushButton("Export to CSV")
        export_btn.clicked.connect(self.export_to_csv)
        button_layout.addWidget(export_btn)
        button_layout.addStretch()
        layout.addLayout(button_layout)

        self.setLayout(layout)

    def display_results(self, results):
        self.results_table.setRowCount(0)
        
        for i, result in enumerate(results):
            self.results_table.insertRow(i)
            self.results_table.setItem(i, 0, QTableWidgetItem(result['filename']))
            self.results_table.setItem(i, 1, QTableWidgetItem(result['path']))
            self.results_table.setItem(i, 2, QTableWidgetItem(f"{result['score']}%"))
            self.results_table.setItem(i, 3, QTableWidgetItem(result['status']))

        self.status_label.setText(f"Found {len(results)} result(s)")
        self.results_table.resizeColumnsToContents()

    def export_to_csv(self):
        if self.results_table.rowCount() == 0:
            QMessageBox.warning(self, "Export", "No results to export")
            return

        filename, _ = QFileDialog.getSaveFileName(self, "Export Results", "", "CSV Files (*.csv)")
        if not filename:
            return

        try:
            with open(filename, 'w') as f:
                f.write("Filename,Path,Score,Status\n")
                for row in range(self.results_table.rowCount()):
                    cols = [self.results_table.item(row, col).text() for col in range(4)]
                    f.write(",".join(f'"{col}"' for col in cols) + "\n")
            
            QMessageBox.information(self, "Export Successful", f"Exported to: {filename}")
        except Exception as e:
            QMessageBox.critical(self, "Export Error", f"Failed to export: {str(e)}")

class DashboardWindow(QMainWindow):
    """Main application window"""
    
    def __init__(self):
        super().__init__()
        self.search_thread = None
        self.init_ui()
        self.setWindowTitle("Content Fingerprinting Dashboard")
        self.setGeometry(100, 100, 1200, 700)

    def init_ui(self):
        # Central widget
        central_widget = QWidget()
        main_layout = QVBoxLayout(central_widget)

        # Search panel
        main_layout.addWidget(QLabel("=== Search Configuration ==="))
        self.search_panel = SearchPanelWidget()
        main_layout.addWidget(self.search_panel)

        # Results panel
        main_layout.addWidget(QLabel("=== Search Results ==="))
        self.results_panel = ResultsPanelWidget()
        main_layout.addWidget(self.results_panel)

        # Button layout
        button_layout = QHBoxLayout()
        
        search_btn = QPushButton("Search")
        search_btn.setStyleSheet("background-color: #4CAF50; color: white; padding: 8px;")
        search_btn.clicked.connect(self.on_search)
        button_layout.addWidget(search_btn)

        clear_btn = QPushButton("Clear")
        clear_btn.setStyleSheet("background-color: #f44336; color: white; padding: 8px;")
        clear_btn.clicked.connect(self.on_clear)
        button_layout.addWidget(clear_btn)

        button_layout.addStretch()
        main_layout.addLayout(button_layout)

        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)

        # Menu bar
        self.setup_menu_bar()

        # Status bar
        self.statusBar().showMessage("Ready")

    def setup_menu_bar(self):
        menubar = self.menuBar()
        
        # File menu
        file_menu = menubar.addMenu("&File")
        exit_action = file_menu.addAction("E&xit")
        exit_action.triggered.connect(self.close)

        # Help menu
        help_menu = menubar.addMenu("&Help")
        about_action = help_menu.addAction("&About")
        about_action.triggered.connect(self.show_about)

    def on_search(self):
        query = self.search_panel.get_query()
        
        if not query:
            QMessageBox.warning(self, "Input Error", "Please enter a search query")
            return

        if len(query) < 2:
            QMessageBox.warning(self, "Input Error", "Query must be at least 2 characters")
            return

        protected_dir = self.search_panel.get_directory()
        if not os.path.exists(protected_dir):
            QMessageBox.warning(self, "Directory Error", f"Directory not found: {protected_dir}")
            return

        self.statusBar().showMessage("Searching...")
        self.results_panel.progress_bar.setVisible(True)

        threshold = self.search_panel.get_threshold()
        file_type = self.search_panel.get_file_type()

        # Start search in background thread
        self.search_thread = SearchWorker(query, protected_dir, threshold, file_type)
        self.search_thread.finished.connect(self.on_search_finished)
        self.search_thread.error.connect(self.on_search_error)
        self.search_thread.start()

    def on_search_finished(self, results):
        self.results_panel.display_results(results)
        self.statusBar().showMessage(f"Search complete - Found {len(results)} result(s)")
        self.results_panel.progress_bar.setVisible(False)

    def on_search_error(self, error):
        QMessageBox.critical(self, "Search Error", error)
        self.statusBar().showMessage("Search failed")
        self.results_panel.progress_bar.setVisible(False)

    def on_clear(self):
        self.search_panel.reset()
        self.results_panel.display_results([])
        self.statusBar().showMessage("Cleared")

    def show_about(self):
        QMessageBox.about(self, "About Dashboard",
            "Content Fingerprinting & File Search Dashboard\n\n"
            "Version: 2.0 (Python GUI)\n"
            "Purpose: Search and analyze files using fuzzy text matching\n"
            "Author: Kashish\n\n"
            "Built with PyQt5")

def main():
    app = QApplication(sys.argv)
    window = DashboardWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
