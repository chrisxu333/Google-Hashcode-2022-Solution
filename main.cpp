#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>


//#define DEBUG

struct Skill{
    int level;
    std::string name;
};

struct Contributor{
    int id;
    std::string name;
    int status;
    std::vector<Skill> skills;
    std::vector<int> updated;
};

struct Role{
    int requiredLevel;
    std::string name;
};

struct Project{
    int pid;
    std::string name;
    int duration;
    int scoreawarded;
    int bestbefore;
    std::vector<Role> roles;
    std::vector<int> occupied;
    std::vector<int> contrib;
    int status;
    int startDay = 0;
    int endDay = 0;
};


bool cmp(Project& lhs, Project& rhs){
    return lhs.bestbefore < rhs.bestbefore;
}

std::vector<Contributor> totalContrib;
std::vector<Project> totalProject;
std::vector<int> totalPid;
int remainingProject;
int globalAward;

bool assignToProject(Contributor& contributor, Project& project){
    for(auto& i : project.contrib){
        if(i != -1){
            if(contributor.name == totalContrib[i].name){
                return false;
            }
        }
    }
    for(int i = 0; i < project.roles.size(); ++i){
        Role r = project.roles[i];
        for(int j = 0; j < contributor.skills.size(); ++j){
            Skill cr = contributor.skills[j];
            if((project.occupied[i] == 0) && (r.name == cr.name) && (cr.level >= r.requiredLevel)){
                // assign this contributor to the project
                contributor.status = 1;
                project.occupied[i] = 1;
                project.contrib[i] = contributor.id;
                // increase level
                if(cr.level == r.requiredLevel) {
                    contributor.updated[j] = 1;
                }
                return true;
            }
        }
    }
    return false;
}

bool ableToStart(const Project& p){
    if(std::all_of(p.occupied.begin(), p.occupied.end(), [](int x) { return x==1; })){
        return true;
    }
    return false;
}

void revertState(Contributor& c){
    c.status = 0;
    for(auto& i : c.updated) i = 0;
}

void startRunning(){
    int currentDay = 0;
    // sort total projects based on best-before day.
    std::sort(totalProject.begin(), totalProject.end(), cmp);
    while(true){
#ifdef DEBUG
        std::cout << "Day " << currentDay << std::endl;
        std::cout << "Remaining project: " << remainingProject << std::endl;
#endif
        if(remainingProject == 0 || currentDay > 5000){
            break;
        }
        // try all projects in each iteration
        for(int i = 0; i < totalProject.size(); ++i) {
            // if the project is not started, try it
            if (totalProject[i].status == 0) {
                // try to assign all contributor to the project
                std::vector<int> tmpContainer;
                for (int j = 0; j < totalContrib.size(); ++j) {
                    if(totalContrib[j].status == 0){
                             assignToProject(totalContrib[j], totalProject[i]);
                             tmpContainer.push_back(j);
                    }
                }
                // if the project satisfies its requirement, then start it.
                if(ableToStart(totalProject[i])){
                    totalProject[i].status = 1;
                    totalProject[i].startDay = currentDay;
                    tmpContainer.clear();
                }else{
                    // reset all contributors status to 0;
                    for(auto & j : tmpContainer){
                        totalContrib[j].status = 0;
                    }
                    tmpContainer.clear();
                    for(auto& qq : totalProject[i].occupied){
                        qq = 0;
                    }
                    for(auto& cc : totalProject[i].contrib){
                        if(cc != -1){
                            for(int & up : totalContrib[cc].updated){
                                up = 0;
                            }
                        }
                    }
                    for(auto& k : totalProject[i].contrib){
                        k = -1;
                    }
#ifdef DEBUG
                    for(auto& j : totalContrib){
                        std::cout << j.name << j.status << std::endl;
                    }
                    std::cout << "proj: ";
                    for(auto& j: totalProject){
                        std::cout << j.name << j.status << std::endl;
                        for(int k = 0; k < j.occupied.size(); ++k){
                            std::cout << j.roles[k].name << ": " << j.occupied[k] << std::endl;
                        }
                    }
#endif
                }
            }else if(totalProject[i].status == 1){  // the project has started, determine whether it has finished
                if(currentDay - totalProject[i].startDay >= totalProject[i].duration){
                    // put projects in the order of their finish time
                    totalProject[i].status = 2;
                    totalProject[i].endDay = currentDay - 1;
                    totalPid.push_back(i);
                    for(auto& c : totalProject[i].contrib){
                        totalContrib[c].status = 0;
                    }
                    int totalAward = 0;
                    if(totalProject[i].endDay < totalProject[i].bestbefore){
                        totalAward = totalProject[i].scoreawarded;
                    }else{
                        totalAward = totalProject[i].scoreawarded - (totalProject[i].endDay - totalProject[i].bestbefore);
                    }
                    //std::cout << totalAward << std::endl;
                    globalAward += totalAward;
                    remainingProject--;
                    // increase level
                    for(auto& cc : totalProject[i].contrib){
                        for(int up = 0; up < totalContrib[cc].updated.size(); ++up){
                            if(totalContrib[cc].updated[up] == 1){
                                //totalContrib[cc].skills[up].level++;
                                totalContrib[cc].updated[up] = 0;
                            }
                        }
                    }
                }
            }
        }
        // after trying, increase currentDay by 1.
        currentDay++;
        //std::cout << currentDay << std::endl;
    }
}

void output(const std::string& testOut){
    std::ofstream out;
    out.open(testOut);
    out << totalPid.size() << "\n";
    for(auto& pid : totalPid){
        if(totalProject[pid].status == 2){
            // finished project
            out << totalProject[pid].name << "\n";
            out << totalContrib[totalProject[pid].contrib[0]].name;
            for(int i = 1; i < totalProject[pid].contrib.size(); ++i){
                 out << " " << totalContrib[totalProject[pid].contrib[i]].name;
            }
            out << "\n";
        }
    }
    out.close();
}

void input(const std::string& testIn){
    std::ifstream in;
    in.open(testIn);
    // read number of contributors and projects
    int numContrib, numProject;
    in >> numContrib >> numProject;
    totalContrib.resize(numContrib);
    totalProject.resize(numProject);
    // read each contributor;
    for(int i = 0; i < numContrib; ++i){
        Contributor c;
        int numSkills;
        c.id = i;
        c.status = 0;
        in >> c.name >> numSkills;
        c.skills.resize(numSkills);
        c.updated.resize(numSkills);
        for(int j = 0; j < numSkills; ++j){
            Skill s;
            in >> s.name >> s.level;
            c.skills[j] = s;
        }
        totalContrib[i] = c;
    }
    for(int i = 0; i < numProject; ++i){
        Project p;
        int numRoles;
        p.status = 0;
        in >> p.name >> p.duration >> p.scoreawarded >> p.bestbefore >> numRoles;
        p.occupied.resize(numRoles);
        p.roles.resize(numRoles);
        p.contrib.resize(numRoles);
        for(int & q : p.contrib){
            q = -1;
        }
        p.pid = i;
        for(int j = 0; j < numRoles; ++j){
            Role r;
            in >> r.name >> r.requiredLevel;
            p.roles[j] = r;
        }
        totalProject[i] = p;
    }
    remainingProject = numProject;
    in.close();
}

int main() {
//    std::string IN[6] = {};
//    std::string OUT[6] = {};
    std::string testIn = R"(D:\pr-assignment\teamwork\c_collaboration.in.txt)";
    std::string testOut = R"(D:\pr-assignment\teamwork\c_collaboration.out.txt)";
    globalAward = 0;
    input(testIn);
    startRunning();
    std::cout << globalAward << std::endl;
    output(testOut);
    return 0;
}
