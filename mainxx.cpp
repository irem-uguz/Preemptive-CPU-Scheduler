//Written by Irem Uguz. 25.11.2018
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <map>
#include <iterator>
#include <vector>
#include <queue>
#include <iomanip>

using namespace std;

class Process {
public:
    string processName;
    string fileName;
    //currentLine is the next line to be executed.
    int currentLine;
    int arrivalTime;
    int waitingTime;
    int finishingTime;
    int priority;
    //Last executed time is used to determine the total waiting time.
    int lastExecutedTime;

    Process(std::string processName, int arrivalTime, int priority, std::string fileName) {
        this->processName = processName;
        this->fileName = fileName;
        this->arrivalTime = arrivalTime;
        this->priority = priority;
        this->currentLine = 0;
        this->waitingTime = 0;
        this->finishingTime = 0;
        this->lastExecutedTime = arrivalTime;
    }

    //setLastExecutedTime updates the waiting time and sets the last executed time.
    void setLastExecutedTime(int currentTime) {
        this->waitingTime += (currentTime - lastExecutedTime);
        this->lastExecutedTime = currentTime;
    }

    int totalWaitingTime() { return waitingTime; }

    int turnaroundTime() { return finishingTime - arrivalTime; }

    Process(const Process &x) {
        this->processName = x.processName;
        this->fileName = x.fileName;
        this->arrivalTime = x.arrivalTime;
        this->priority = x.priority;
        this->currentLine = x.currentLine;
        this->waitingTime = x.waitingTime;
        this->finishingTime = x.finishingTime;
        this->lastExecutedTime = x.lastExecutedTime;
    }

    Process &operator=(const Process &x) {
        this->processName = x.processName;
        this->fileName = x.fileName;
        this->arrivalTime = x.arrivalTime;
        this->priority = x.priority;
        this->currentLine = x.currentLine;
        this->waitingTime = x.waitingTime;
        this->finishingTime = x.finishingTime;
        this->lastExecutedTime = x.lastExecutedTime;
    }
};


struct CompareArrivalTime {
    bool operator()(Process* const p1, Process* const p2) {
        // return "true" if "p1" has higher priority than "p2"
        if (p1->priority > p2->priority) {
            return true;
        } else if (p1->priority == p2->priority) {
            return p1->arrivalTime >= p2->arrivalTime;
        } else {
            return false;
        }
    }
};
//Prints the ready queue, because it is passed by value, the queue in main is not affected.
void printQueue(priority_queue<Process*, vector<Process*>, CompareArrivalTime> readyQueue, int currentTime) {
    printf("%d:HEAD-", currentTime);
    if(readyQueue.empty()){
        printf("-TAIL\n");
        return;
    }
    while (!readyQueue.empty()) {
        Process* p = readyQueue.top();
        readyQueue.pop();
        printf("%s[%d]-", p->processName.c_str(), p->currentLine + 1);
    }
    printf("TAIL\n");
}


int main() {
    int currentTime = 0;
    //Priority queue that stores the ready processes.
    priority_queue<Process*, vector<Process*>, CompareArrivalTime> readyQueue;
    //arrival queue and process queue stores the processes in the order that they arrived.
    queue<Process*> arrivalQueue;
    queue<Process*> processQueue;
    //Map for code files. A vector that contains the instructions of the file is stored birth the name of the code file.
    map<string, vector<int> > fileMap;
    ifstream infile("./definition.txt");
    string line;
    freopen("./output.txt", "w", stdout);
    //Code files are added to fileMap by their names, and istructions and their execution times are stored as vectors. 
    for(int i=1;i<=4;i++) {
        stringstream ss;
        ss << "./code" << i << ".txt";
        string codeName=ss.str();
        ifstream codeRead(codeName.c_str());
        queue<int> tempInstr;
        string instruction;
        int lineCount = 0;
        while (getline(codeRead, instruction)) {
            lineCount++;
            string temp;
            int time;
            istringstream css(instruction);
            if (!(css >> temp >> time)) { return -1; }
            tempInstr.push(time);
	}
        vector<int> instr(lineCount);
        for (int i = 0; i < lineCount; i++) {
            instr[i] = tempInstr.front();
            tempInstr.pop();
        }
        //How much time each line takes is in the instr vector.
        fileMap[codeName] = instr;
    }

    //Process the definition file line by line
    while (getline(infile, line)) {
        string processName, fileName;
        int priority, arrivalTime;
        istringstream iss(line);
        //Return if there is error in line format
        if (!(iss >> processName >> priority >> fileName >> arrivalTime)) { return -1; }
        fileName = "./" + fileName + ".txt";
        //Push the process to arrival queue and process queue.
	Process* pp=new Process(processName, arrivalTime, priority, fileName);
	arrivalQueue.push(pp);
	processQueue.push(pp);
    }
    //Here, all the processes are added to arrival queue.
    //Now we pop the first arrived process and add it do ready queue.
    printQueue(readyQueue,currentTime);
    readyQueue.push(arrivalQueue.front());
    arrivalQueue.pop();
    currentTime = readyQueue.top()->arrivalTime;
    //If there are multiple processes that arrived at the same time, we add them too.
    while (!arrivalQueue.empty() && readyQueue.top()->arrivalTime == arrivalQueue.front()->arrivalTime) {
        readyQueue.push((arrivalQueue.front()));
        arrivalQueue.pop();
    }
    printQueue(readyQueue,currentTime);

    while (!readyQueue.empty() || !arrivalQueue.empty()) {
        bool print = false;
        if (!readyQueue.empty()) {
	    //When there is a process in ready queue, its next instruction is executed.
            Process* p = readyQueue.top();
            readyQueue.pop();
	    //p's waiting time is updated with setLastExecutedTime()
            p->setLastExecutedTime(currentTime);
            currentTime += fileMap[p->fileName][p->currentLine];
            p->currentLine++;
	    //print is true if the process is finished executing.
            print = fileMap[p->fileName].size()<=p->currentLine;
            if (print) {
                p->finishingTime = currentTime;
                //readyQueue.pop();
            } else {
		readyQueue.push(p);
                p->lastExecutedTime = currentTime;
            }
        }else if(!arrivalQueue.empty()){
	    //When the ready queue is empty, we add the next process in arrival queue to ready queue.
	    print=true;
	    Process* pp=arrivalQueue.front();
	    readyQueue.push(pp);
	    currentTime=pp->arrivalTime;
	    arrivalQueue.pop();
	}
        while (!arrivalQueue.empty() && arrivalQueue.front()->arrivalTime <= currentTime) {
	    //If there are arrived processes in arrival queue, before the current time, it is added to ready queue.
            print = true;
            readyQueue.push(arrivalQueue.front());
            currentTime=arrivalQueue.front()->arrivalTime <= currentTime ? currentTime : arrivalQueue.front()->arrivalTime;
            arrivalQueue.pop();
        }
        if (print) {
	    //Ready queue is printed when necessary.
            printQueue(readyQueue, currentTime);
        }
    }
    printf("\n");
    while(!processQueue.empty()){
	//Every process, their turnaroun time and total waiting times are printed.
	Process* p=processQueue.front();
	processQueue.pop();
	printf("Turnaround time for %s = %d ms\n",p->processName.c_str(),p->turnaroundTime());
	printf("Waiting time for %s = %d\n",p->processName.c_str(),p->totalWaitingTime());
    }
    return 0;
}
