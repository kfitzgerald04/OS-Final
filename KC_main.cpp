/**
* Kendra Fitzgerald + Craig Crutcher
 * OS Final Project | Page Replacement (FIFO & OPT)
 * CSCI 4300
 *
 * The program:
 *   1. Reads algorithm type (F or O)
 *   2. Reads number of frames
 *   3. Reads the page reference string
 *   4. Runs FIFO or OPT
 *   5. Prints the table & total page faults
 * 
 * To Run: Ctrl + ~ to open the terminal -> cl /EHcs KC_main.cpp creates an .exe
 * in case it's ran on a different machine -> .\KC_main.exe allows you to run the program
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

using namespace std;

const int MAX_REFS = 1000;   // maximum reference length
const int MAX_FRAMES = 20;   // maximum number of frames

/* ============================================================================
   readFile
   Reads:
       Algorithm,Frames,ref1,ref2,ref3,...
   ============================================================================ */

bool readFile(const string& filename, char& algorithm, int& frames, int refs[], int& refCount)
{
    ifstream file(filename);
    if (!file)
    {
        cout << "Error: Could not open file.\n";
        return false;
    }

    string line;
    getline(file, line);
    stringstream ss(line); // allows for splitting the line by commas

    string letter;

    // Get algorithm letter
    getline(ss, letter, ','); // read up to the first comma into letter
    algorithm = letter[0];

    // Get number of frames
    getline(ss, letter, ','); // read the next token
    frames = stoi(letter);

    // Get each reference integer
    refCount = 0;
    while (getline(ss, letter, ',')) // for each remaining token (ref string), stoi, store in refs, ++ count
    {
        refs[refCount++] = stoi(letter);
    }

    return true;
}

/* ============================================================================
   printTable
   Prints the reference string, the table of memory states, and total faults.
   ============================================================================ */

void printTable(int refs[], int refCount, int table[MAX_FRAMES][MAX_REFS], int frames,int pageFaults)
{
    // Print reference string across top (width of 5 for readability)
    for (int i = 0; i < refCount; i++)
        cout << setw(5) << refs[i];
    cout << "\n";

    // Print dashed line (---------)
    cout << string(refCount * 5, '-') << "\n";

    // Print table rows
    for (int r = 0; r < frames; r++) // for each frame (row), print the value in each col (ref)
    {
        for (int c = 0; c < refCount; c++)
        {
            if (table[r][c] == -1) // if the table cell is -1, print a blank
                cout << setw(5) << " ";
            else
                cout << setw(5) << table[r][c];
        }
        cout << "\n";
    }
    
    // Print total page faults
    cout << "\nTotal Page Faults = " << pageFaults << "\n";
}

/* ============================================================================
   writeCol
   Writes the current memory into column "col".
   ============================================================================ */

void writeCol(int table[MAX_FRAMES][MAX_REFS], int memory[], int frames, int col)
{
    for (int i = 0; i < frames; i++)
    {
        // copies the current memory[] into a column of the table
        table[i][col] = memory[i];   // store each frame
    }
}

/* ============================================================================
   FIFO 
   Uses:
     - memory[]: stores pages in frames
     - next: index of the next frame to replace
   ============================================================================ */

void FIFO(int refs[], int refCount, int frames, int table[MAX_FRAMES][MAX_REFS], int& pageFaults)
{
    // Initialize table with -1 so empty cells are blank
    for (int r = 0; r < frames; r++)
        for (int c = 0; c < refCount; c++)
            table[r][c] = -1;

    int memory[MAX_FRAMES]; // will gold the current pages in frames
    int memCount = 0;     // how many frames are currently used
    pageFaults = 0;
    int next = 0;         // points to the next frame idex FIFO

    // Initialize memory as empty
    for (int i = 0; i < frames; i++)
        memory[i] = -1;

    for (int col = 0; col < refCount; col++) 
    {
        int page = refs[col]; // for each page ref (col)

        // check if page is already in memory
        bool inMem = false;
        for (int i = 0; i < memCount; i++)
        {
            if (memory[i] == page)
            {
                inMem = true;
                break;
            }
        }

        if (!inMem)
        {
            // Page fault occurs
            pageFaults++;

            if (memCount < frames) // if there is still an empty frame
            {
                // memory not full =  place in next empty slot
                memory[memCount++] = page;
            }
            else
            {
                // memory full =  replace oldest using FIFO pointer (next)
                memory[next] = page;
                next = (next + 1) % frames;   // wrap around, advance next 
            }

            // Record the memory state for this column
            writeCol(table, memory, frames, col); // saving the current memory into the table
        }
    }
}

/* ============================================================================
   OPT 
   Uses:
     - memory[]: holds the pages
   Uses:
     - On page fault when memory is full, look ahead from (col+1) to find next
       use for each page in memory; choose the page with the farthest next use
   ============================================================================ */

void OPT(int refs[], int refCount, int frames, int table[MAX_FRAMES][MAX_REFS], int& pageFaults)
{
    // initialize table with -1
    for (int r = 0; r < frames; r++)
        for (int c = 0; c < refCount; c++)
            table[r][c] = -1;

    int memory[MAX_FRAMES];
    int memCount = 0;
    pageFaults = 0;

    //iInitialize frames as empty
    for (int i = 0; i < frames; i++)
        memory[i] = -1;

    for (int col = 0; col < refCount; col++)
    {
        int page = refs[col];
        bool inMem = false;

        // check if page is already in memory
        for (int i = 0; i < memCount; i++)
        {
            if (memory[i] == page)
            {
                inMem = true;
                break;
            }
        }

        if (!inMem)
        {
            // page fault
            pageFaults++;

            if (memCount < frames)
            {
                // still space, place page in next empty slot
                memory[memCount++] = page;
            }
            else
            {
                // when the memory is full use OPT to drop a page
                int farthestIndex = -1;
                int farthestDistance = -1;

                // for each page in memory, find next use distance
                for (int i = 0; i < frames; i++)
                {
                    int nextUse = -1; // -1 => not found in future
                    for (int k = col + 1; k < refCount; k++)
                    {
                        if (refs[k] == memory[i])
                        {
                            nextUse = k;
                            break;
                        }
                    }

                    if (nextUse == -1)
                    {
                        // drop the page 
                        farthestIndex = i;
                        farthestDistance = INT_MAX; // mark as infinite distance
                        break;
                    }
                    else
                    {
                        // distance is nextUse
                        if (nextUse > farthestDistance)
                        {
                            farthestDistance = nextUse;
                            farthestIndex = i;
                        }
                    }
                }

                // replace chosen frame
                if (farthestIndex == -1)
                {
                    // debug
                    farthestIndex = 0;
                }
                memory[farthestIndex] = page;
            }

            // write to memory
            writeCol(table, memory, frames, col);
        }
    }
}


/* ============================================================================
   MAIN
   ============================================================================ */

int main()
{
    char algorithm;
    int frames;
    int refs[MAX_REFS];
    int refCount;

    // prompt user to enter filename
    string filename;
    cout << "Enter input filename: ";
    cin >> filename;

    // read in the file
    if (!readFile(filename, algorithm, frames, refs, refCount))
        return 1;

    int table[MAX_FRAMES][MAX_REFS];
    int pageFaults = 0;

    // if algorithm is F, run FIFO
    if (algorithm == 'F')
    {
        cout << "Running FIFO...\n";
        FIFO(refs, refCount, frames, table, pageFaults);
    }
    // else if algorithm is O, run OPT
    else if (algorithm == 'O')
    {
        cout << "Running OPT...\n";
        OPT(refs, refCount, frames, table, pageFaults);
    }
    // if something other than F or O, error
    else
    {
        cout << "Error: Only F and O algorithms supported in this version.\n";
        return 1;
    }

    // print the formatted table 
    printTable(refs, refCount, table, frames, pageFaults);
    return 0;
}
