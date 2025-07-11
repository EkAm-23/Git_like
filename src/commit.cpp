#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "../include/repository.h"
#include "../include/branch.h"
#include "iostream"

namespace fs = std::filesystem;

using namespace std;

// print names of files in the staging area
void lsFiles()
{
    // read the index file
    fs::path refPath(".gitlike/index");

    // read the entire data of the index file
    ifstream f(refPath);
    string data;
    {
        stringstream buffer;
        buffer << f.rdbuf();
        data = buffer.str();
    }
    f.close();
}

string getParentCommitHash(string commitHash)
{
    // get the content of the commit object (call catFile())
    string commitContent = catFile(commitHash);

    // go to the second line and extract the parent hash
    
    // extract the second line
    int secondLineStart = commitContent.find("\n") + 1;
    int secondLineEnd = commitContent.find("\n", secondLineStart);
    commitContent = commitContent.substr(secondLineStart, secondLineEnd-secondLineStart - 1);

    // remove the "parent " text
    if(commitContent.length() < 7)
    {
        return "0";
    }
    string parentHash = commitContent.substr(7);
    parentHash = parentHash.substr(0, parentHash.find(" "));

    return parentHash;
}

void log()
{
    string directoryPath = "./.gitlike/refs/heads";
    // iterate through all branches
    for (const auto& entry : fs::directory_iterator(directoryPath))
    {
        // if it's a regular file
        if (entry.is_regular_file())
        {
            // open the file
            ifstream file(entry.path());
            if (file.is_open())
            {
                string line;
                getline(file, line);

                // get the commit's hash
                string commitHash = refResolver("refs/heads/" + entry.path().filename().string());
                string commitContent = catFile(commitHash);
                
                // print this commit's content
                cout<<commitContent<<endl;
                cout<<endl;

                // iterate to find the previous commit until the initial one is reached
                while(true)
                {
                    commitHash = getParentCommitHash(commitHash);
                    if(commitHash == "0")
                    {
                        break;
                    }
                    commitContent = catFile(commitHash);
                    cout<<commitContent<<endl;
                    cout<<endl;
                }
                file.close();
            }
            else
            {
                cerr << "Failed to open file: " << entry.path() << endl;
            }

            cout << "---------------------------------\n";
        }
    }
}

string createTreeObject(string path)
{
    string treeContent;

    // traverse the directory
    fs::path workingDir(path);

    // for each file, read the file
    for (const auto & entry : fs::directory_iterator(workingDir))
    {
        // retrieve the file mode, it's path and compute it's SHA-1 hash
        string entryPath = entry.path().string();
        // string entryName = entry.path().filename().string();

        // FIX
        string entryMode = entry.is_directory() ? "040000" : "100644";

        // make a record to store in the tree object in the format: [mode] space [path] 0x00 [sha-1]

        // for a folder, call this function recursively
        if (entry.is_directory())
        {
            // don't include git's internal files for the tree
            if(entry.path() == "./.gitlike" || entry.path() == "./.git") { continue; }

            string subtreeHash = createTreeObject(entry.path());
            treeContent += entryMode + " " + entryPath + '\0' + subtreeHash + '\n';
        }
        // for each file, read the file and create a blob object
        else if (entry.is_regular_file())
        {
            string blobHash = hashObject(entry.path());
            treeContent += entryMode + " " + entryPath + '\0' + blobHash + '\n';
        }
    }

    // create the tree object content with header
    string header = "tree " + to_string(treeContent.size()) + '\0';
    string treeData = header + treeContent;

    // create a file to store the records
    ofstream f("treeObjDummy.txt");
    
    // store all the records in a file
    f << treeData;
    f.close();

    // create a tree git-object for it
    string treeHash = hashObject("treeObjDummy.txt");

    // delete the original file
    fs::remove("treeObjDummy.txt");

    // return own's hash received by hashObject()
    return treeHash;
}

void recreateTree(string treeContent)
{
    // create an input string stream
    istringstream stream(treeContent);
    string line;

    // read each line from the stream
    while (getline(stream, line))
    {
        if(line.find("tree ") == 0)
        {
            continue;
        }
        // if it is a blob, create a file in the working directory
        if(line.find("100644") == 0)
        {
            // get the hash of the object
            string hash = line.substr(line.find('\0') + 1);
            // get the uncompressed content of the blob file
            string content = catFile(hash);

            // get the path of the file from the object
            string filePath = line.substr(line.find(" ")+1, line.find('\0')-line.find(" ")-1);

            // create required directory
            fs::create_directories(filePath.substr(0, filePath.find_last_of("/")));

            // create a new file with the name retrieved from tree
            ofstream f(filePath);

            // put the contents into the new file created
            f << content;
            f.close();
        }

        // if it is another tree object, create a directory and recursively parse the tree object
        if(line.find("040000") == 0)
        {
            // get the hash of the object
            string hash = line.substr(line.find('\0') + 1);
            // get the uncompressed content of the blob file
            string tree = catFile(hash);
            
            // call recursively to parse the tree and recreate directory
            recreateTree(tree);
        }
    }
}

// function to restore a directory to a commit
void checkout(string branchName)
{
    // FIX
    string commitHash = refResolver("refs/heads/master");

    // get the commit git-object from the object database
    string commit = catFile(commitHash);

    // retrieve the tree object hash from the commit object
    string treeHash = commit.substr(5, 40);

    // get the uncompressed content of the tree git-object
    string tree = catFile(treeHash);

    // recreate directory structure
    recreateTree(tree);

    // update index (staging area)
    // update the HEAD reference
    ofstream file("./.gitlike/HEAD");
    file << "ref: refs/heads/" + branchName;
    file.close();
}

void createCommit(string commitMessage)
{
    // find the working directory
    string path = "./";

    // get a list of all the modified files from the staging area

    // for each modified file, compute the SHA-1 hash (call hashObject())

    // update the index file?

    string commitContent;

    // create a tree object representing the working directory
    string treeHash = createTreeObject(path);

    // get parent as the current commit with HEAD
    ifstream parent("./.gitlike/HEAD");
    string data;
    {
        stringstream buffer;
        buffer << parent.rdbuf();
        data = buffer.str();
    }
    parent.close();
    string parentHash = refResolver(data);

    // write commit metadata (tree hash, parent, author, committer, timestamp, message)
    commitContent += "tree " + treeHash + "\n";
    commitContent += "parent " + parentHash + "\n";
    commitContent += commitMessage + "\n";

    // create a file to store the records
    ofstream f("commitObjDummy.txt");
    
    // store all the records in a file
    f << commitContent;
    f.close();

    // create a commit git-object for it (call hashObject())
    string commitHash = hashObject("commitObjDummy.txt");

    // delete the original file
    fs::remove("commitObjDummy.txt");

    // update the branch ref to the new commit's hash
    ifstream branch("./.gitlike/HEAD");
    string d;
    
    stringstream b;
    b << branch.rdbuf();
    d = b.str();

    branch.close();

    string branchName = d.substr(16);

    branchName.erase(branchName.find_last_not_of(" \n\r\t") + 1);

    // update the branch ref
    ofstream branchHash("./.gitlike/refs/heads/" + branchName);
    branchHash << commitHash;
    branchHash.close();
}

// int main(int argc, char const *argv[])
// {
//     createCommit("testing 123");
//     // checkout("master");
//     // log();
//     return 0;
// }