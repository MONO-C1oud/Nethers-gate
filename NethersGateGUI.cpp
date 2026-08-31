#include "liberaries.h"
#include "variableExtraction.h"
#include "metamorphism.h"
#include "garbageCodeInsert.h"
#include "ImportObfuscation.h"
#include "shellcodeEncoder.h"
#include "walkthroughhellsgate.h"
#include "ApplySandboxEvasion.h"
#include "NethersGateGUI.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QStandardItem>

// Constructor
NethersGateGUI::NethersGateGUI(QWidget* parent)
    : QMainWindow(parent),
    filesListModel(new QStandardItemModel(this)),
    binFilesListModel(new QStandardItemModel(this))
{
    ui.setupUi(this);

    // Set model for QListView
    // Initialize UI elements
    ui.SelectedFilesList->setModel(filesListModel);
    ui.SelectBinFileList->setModel(binFilesListModel);
    ui.SelectBinFileButton->setEnabled(false);
    ui.SelectBinFileList->setEnabled(false);

    // Connect signals to slots
    connect(ui.checkBoxShellcodeEncryption, &QCheckBox::toggled, this, &NethersGateGUI::onExclusiveShellcodeOptionToggled);
    connect(ui.checkBoxHellsGate, &QCheckBox::toggled, this, &NethersGateGUI::onExclusiveShellcodeOptionToggled);
    connect(ui.checkBoxHellsHall, &QCheckBox::toggled, this, &NethersGateGUI::onExclusiveShellcodeOptionToggled);
    connect(ui.SelectBinFileButton, &QPushButton::clicked, this, &NethersGateGUI::onSelectBinFilesClicked);
    connect(ui.SelectFilesButton, &QPushButton::clicked, this, &NethersGateGUI::onSelectFilesClicked);
    connect(ui.EvadeButton, &QPushButton::clicked, this, &NethersGateGUI::onEvadeClicked);
}

// Destructor
NethersGateGUI::~NethersGateGUI()
{}

void NethersGateGUI::onExclusiveShellcodeOptionToggled()
{
    // Determine which checkbox is checked
    bool isShellcode = ui.checkBoxShellcodeEncryption->isChecked();
    bool isHellsGate = ui.checkBoxHellsGate->isChecked();
    bool isHellsHall = ui.checkBoxHellsHall->isChecked();

    // Enforce mutual exclusivity
    if (sender() == ui.checkBoxShellcodeEncryption && isShellcode) {
        ui.checkBoxHellsGate->setChecked(false);
        ui.checkBoxHellsHall->setChecked(false);
    }
    else if (sender() == ui.checkBoxHellsGate && isHellsGate) {
        ui.checkBoxShellcodeEncryption->setChecked(false);
        ui.checkBoxHellsHall->setChecked(false);
    }
    else if (sender() == ui.checkBoxHellsHall && isHellsHall) {
        ui.checkBoxShellcodeEncryption->setChecked(false);
        ui.checkBoxHellsGate->setChecked(false);
    }

    // Enable or disable bin file selection
    bool anyChecked = isShellcode || isHellsGate || isHellsHall;
    ui.SelectBinFileButton->setEnabled(anyChecked);
    ui.SelectBinFileList->setEnabled(anyChecked);

    // Clear list if none selected
    if (!anyChecked) {
        binFilesListModel->clear();
        selectedBinFile.clear();
    }
}


void NethersGateGUI::onSelectBinFilesClicked() {
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select Shellcode Binary Files",
        "",
        "Binary Files (*.bin)"
    );

    if (!files.isEmpty()) {
        binFilesListModel->clear();
        selectedBinFile = files.first(); // Store first selected file
        for (const QString& file : files) {
            binFilesListModel->appendRow(new QStandardItem(file));
        }
    }
}

// Slot: Select Files
void NethersGateGUI::onSelectFilesClicked()
{
    selectedFiles = QFileDialog::getOpenFileNames(this, "Select C++ Source Files", "", "C++ Files (*.cpp)");

    if (selectedFiles.isEmpty()) {
        QMessageBox::information(this, "No Files Selected", "Please select at least one source file.");
        return;
    }

    updateFileListView();
}

// Helper: Update QListView
void NethersGateGUI::updateFileListView()
{
    filesListModel->clear();
    for (const QString& file : selectedFiles) {
        QStandardItem* item = new QStandardItem(file);
        filesListModel->appendRow(item);
    }
}

// Slot: Apply Obfuscation Techniques
void NethersGateGUI::onEvadeClicked()
{
    bool isHellsGate = ui.checkBoxHellsGate->isChecked();
    bool isHellsHall = ui.checkBoxHellsHall->isChecked();

    // Allow shellcode-only operations
    if (selectedFiles.isEmpty() && !(isHellsGate || isHellsHall)) {
        QMessageBox::warning(this, "Error", "No files selected for obfuscation.");
        return;
    }

    // Gather the selected obfuscation techniques
    bool boolreplaceVariableNames = ui.checkBoxReplaceVariableNames->isChecked();
    bool boolinsertGarbageCode = ui.checkBoxInsertGarbageCode->isChecked();
    bool boolapplyMetamorphism = ui.checkBoxApplyMetamorphism->isChecked();
    bool boolencryptVariableData = ui.checkBoxEncryptVariableData->isChecked();
    bool boolImportObfuscation = ui.checkBoxImportObfuscation->isChecked();
    bool boolSandboxEvasionTechniques = ui.checkBoxSandboxEvasionTechniques->isChecked();
    bool boolShellcodeEncryption = ui.checkBoxShellcodeEncryption->isChecked();
    bool boolHellsGate = ui.checkBoxHellsGate->isChecked();
    bool boolHellsHall = ui.checkBoxHellsHall->isChecked();

    if (!boolreplaceVariableNames && !boolinsertGarbageCode && !boolapplyMetamorphism &&
        !boolencryptVariableData && !boolImportObfuscation && !boolSandboxEvasionTechniques &&
        !boolShellcodeEncryption && !isHellsGate && !isHellsHall) {
        QMessageBox::warning(this, "Error", "No obfuscation techniques selected.");
        return;
    }

    // Create the "obfuscated" folder if it doesn't exist
    QDir obfuscatedDir("obfuscated");
    if (!obfuscatedDir.exists()) {
        obfuscatedDir.mkdir(".");
    }

    // Apply obfuscation techniques to each selected file
    for (const QString& file : selectedFiles) {
        // Read the C++ source code from the file
        std::ifstream sourceFile(file.toStdString());
        std::string code((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());
        std::string modifiedCode = code;

        // Extract variables and their values
        auto variables = extractVariables(code);
        bool randomExecutionCode = false;

        if (boolShellcodeEncryption) {
            // implement code here
            if (selectedBinFile.isEmpty()) {
                QMessageBox::warning(this, "Error",
                    "Please select a .bin file containing shellcode before using this module.");
                return;
            }

            // Add shellcode encoding
            string shellcodeBin = selectedBinFile.toStdString(); // Configurable path
            ShellcodeStub stub = generateShellcodeStub(shellcodeBin);

            if (!stub.declarations.empty() && !stub.decoding.empty()) {
                // Insert declarations
                size_t declPos = modifiedCode.find("// shellcode declaration");
                if (declPos != string::npos) {
                    size_t endLine = modifiedCode.find('\n', declPos);
                    if (endLine != string::npos) {
                        modifiedCode.insert(endLine + 1, "\n" + stub.declarations);
                    }
                }

                // Insert decoding stub
                size_t decodePos = modifiedCode.find("// decode shellcode");
                if (decodePos != string::npos) {
                    size_t endLine = modifiedCode.find('\n', decodePos);
                    if (endLine != string::npos) {
                        modifiedCode.insert(endLine + 1, "\n" + stub.decoding);
                    }
                }

                // Debug: Print modified code
                cout << "[DEBUG] Modified code preview:\n";
                cout << modifiedCode.substr(0, 1000) << "\n[...]\n";
            }
            else {
                cerr << "[ERROR] Failed to generate shellcode stub" << endl;
            }

            QMessageBox::about(this, "Shellcode Encryption", "Shellcode encryption module run with file: " + selectedBinFile);
        }

        if (boolencryptVariableData) {
            // First, replace variable values in the original code
            modifiedCode = importValueModules(modifiedCode, variables);

            QMessageBox::about(this, "Success", "boolencryptVariableData");     // for testing
        }


        if (boolreplaceVariableNames) {
            // Then, replace variable names in the original code
            modifiedCode = replaceVariableNames(modifiedCode, variables);
        }


        if (boolinsertGarbageCode) {
            int numGarbFunc = 6;
            modifiedCode = injectGarbageCode(modifiedCode, numGarbFunc);
        }



        if (boolapplyMetamorphism) {
            vector<string> functionNames = extractFunctionNames(modifiedCode);
            modifiedCode = applyMetamorphism(modifiedCode, functionNames, randomExecutionCode);    
            randomExecutionCode = true;
        }

        if (boolImportObfuscation) {
            // implement code here
            //Applying the import obfuscation
            modifiedCode = replaceFunctionCalls(modifiedCode);
            modifiedCode = placeFunctionData(modifiedCode);

            QMessageBox::about(this, "Success", "boolImportObfuscation");     // for testing
        }

        if (boolSandboxEvasionTechniques) {
            // implement code here
            ifstream sandboxEvasiveFile("sandboxEvasion.h");
            string headerCode((istreambuf_iterator<char>(sandboxEvasiveFile)), istreambuf_iterator<char>());
            vector<string> evasiveModules = extractSandboxEvasionModules(headerCode);
            modifiedCode = applySandboxEvasion(modifiedCode, evasiveModules, randomExecutionCode);
            randomExecutionCode = true;
            QMessageBox::about(this, "Success", "boolSandboxEvasionTechniques");     // for testing
        }

        // Write the obfuscated code to the output file
        QString outputPath = obfuscatedDir.filePath(QFileInfo(file).fileName());
        std::ofstream outputFile(outputPath.toStdString());
        outputFile << modifiedCode;
    }

    if (boolHellsGate) {
        if (selectedBinFile.isEmpty()) {
            QMessageBox::warning(this, "Error",
                "Please select a .bin file containing shellcode before using this module.");
            return;
        }

        QString sourcePath = "hellsgatesourcefiles/hellsgatemain.c";
        std::ifstream sourceFile(sourcePath.toStdString());
        if (!sourceFile.is_open()) {
            QMessageBox::critical(this, "Error", "Failed to open hellsgatemain.c for reading.");
            return;
        }

        std::string hellsgatecode((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());
        std::string shellcodeBin = selectedBinFile.toStdString();
        HellsGateShellcodeStub stub = HellsGategenerateShellcodeStub(shellcodeBin);

        if (!stub.declarations.empty() && !stub.decoding.empty()) {
            // Insert declarations
            size_t declPos = hellsgatecode.find("// shellcode declaration");
            if (declPos != std::string::npos) {
                size_t endLine = hellsgatecode.find('\n', declPos);
                if (endLine != std::string::npos) {
                    hellsgatecode.insert(endLine + 1, "\n" + stub.declarations);
                }
            }

            // Insert decoding stub
            size_t decodePos = hellsgatecode.find("// decode shellcode");
            if (decodePos != std::string::npos) {
                size_t endLine = hellsgatecode.find('\n', decodePos);
                if (endLine != std::string::npos) {
                    hellsgatecode.insert(endLine + 1, "\n" + stub.decoding);
                }
            }

            // Debug: Print modified code preview
            std::cout << "[DEBUG] Modified HellsGate code preview:\n";
            std::cout << hellsgatecode.substr(0, 1000) << "\n[...]\n";
        }
        else {
            std::cerr << "[ERROR] Failed to generate shellcode stub" << std::endl;
            return;
        }

        // Ensure output directory exists: obfuscated/hellsgatefiles/
        QDir baseDir("obfuscated");
        if (!baseDir.exists()) baseDir.mkdir(".");
        QDir hgDir("obfuscated/hellsgatefiles");
        if (!hgDir.exists()) {
            baseDir.mkdir("hellsgatefiles");
        }

        // Write updated code to obfuscated/hellsgatefiles/main.c
        QString outputPath = "obfuscated/hellsgatefiles/main.c";
        std::ofstream outputFile(outputPath.toStdString());
        outputFile << hellsgatecode;
        outputFile.close();

        // Copy hellsgate.asm and structs.h
        QFile::copy("hellsgatesourcefiles/hellsgate.asm", "obfuscated/hellsgatefiles/hellsgate.asm");
        QFile::copy("hellsgatesourcefiles/structs.h", "obfuscated/hellsgatefiles/structs.h");

        QMessageBox::about(this, "HellsGate", "HellsGate module completed.\nModified main.c and copied required files.");
    }

    if (boolHellsHall) {
        if (selectedBinFile.isEmpty()) {
            QMessageBox::warning(this, "Error",
                "Please select a .bin file containing shellcode before using HellsHall.");
            return;
        }

        // Example stub generation - replace with actual HellsHall logic
        string shellcodeBin = selectedBinFile.toStdString();


        //implementation

        QMessageBox::about(this, "HellsHall", "HellsHall module run with file: " + selectedBinFile);
    }

    // Show success message
    QMessageBox::information(this, "Success", "Obfuscation applied successfully! Files are saved in the 'obfuscated' folder.");
}
