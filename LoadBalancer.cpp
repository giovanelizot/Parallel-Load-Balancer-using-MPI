#include<stdio.h>
#include<map>
#include<vector>
#include<string>
#include<string.h>
#include<fstream>
#include<iostream>
#include<algorithm>
#include <queue>
#include <functional>
#include"mpi.h"
#define num_members 3
using namespace std;
#define SLEEP_TIME 5
#define FACTOR 0.5
#define MAX_ITERATIONS 5
#define RCOMM 0.5
#define LCOMM 0.5

struct core{
	double coreLoad;
	int id;
};

struct task{
	int taskID;
	double taskLoad;

};

vector<task> Tasks;
vector<core> Cores;
map<int,int> Mapping;
map<int,int> New_map;

double latency[20][20];


vector< vector<int> > communication;
int time=0;

void init()
{
	for(int i=0;i<16;i++)
		latency[i][i]=0;
	for(int j=0;j<16;j++){
	for(int i=(j/4)*4;i<((j/4)*4)+4;i++){
		latency[j][i]=0;
		latency[i][j]=0;}
	}
	for(int i=0;i<4;i++){
	latency[i][4]= 0.256;
	latency[4][i]= 0.256;
	latency[i][5]= 0.256;
	latency[5][i]=0.256;
	latency[i][6]=0.256;
	latency[6][i]=0.256;
	latency[i][7]=0.256;
	latency[7][i]=0.256;
	}

		for(int i=0;i<4;i++){
	latency[i][8]= 1.235;
	latency[8][i]=1.235;
	latency[i][9]=1.235;
	latency[9][i]=1.235;
	latency[i][10]=1.235;
	latency[10][i]=1.235;
	latency[i][11]=1.235;
	latency[11][i]=1.235;
	}

		for(int i=0;i<4;i++){
	latency[i][12]= 2.56;
	latency[12][i]=2.56;
	latency[i][13]=2.56;
	latency[13][i]=2.56;
	latency[i][14]=2.56;
	latency[14][i]=2.56;
	latency[i][15]=2.56;
	latency[15][i]=2.56;
	}

	for(int i=4;i<8;i++){
	latency[i][8]= 0.378;
	latency[8][i]=0.378;
	latency[i][9]=0.378;
	latency[9][i]=0.378;
	latency[i][10]=0.378;
	latency[10][i]=0.378;
	latency[i][11]=0.378;
	latency[11][i]=0.378;
	}


	for(int i=4;i<8;i++){
	latency[i][12]=1.563;
	latency[12][i]=1.563;
	latency[i][13]=1.563;
	latency[13][i]=1.563;
	latency[i][14]=1.563;
	latency[14][i]=1.563;
	latency[i][15]=1.563;
	latency[15][i]=1.563;
	}

	for(int i=8;i<12;i++){
	latency[i][12]=0.123;
	latency[12][i]=0.123;
	latency[i][13]=0.123;
	latency[13][i]=0.123;
	latency[i][14]=0.123;
	latency[14][i]=0.123;
	latency[i][15]=0.123;
	latency[15][i]=0.123;
	}


}




void update()
{

	Tasks.erase(Tasks.begin(),Tasks.begin()+Tasks.size());
	Cores.erase(Cores.begin(),Cores.begin()+Cores.size());

	//update Tasks
	stringstream temp;
		temp<<time;
		string a="./TIME/t"+temp.str()+"/taskLoads.txt";
		FILE * fp=fopen(a.c_str(),"r");
		char temp[1000];
		while(fscanf(fp,"%s",&temp)!=EOF)
		{
			char* token=strtok((string)temp,",");
			task a;
			int i=0;
			while(token!=NULL)
			{	
				if(i%3==0)
					a.taskID=atoi(token);
				else if(i%3==1)
					a.taskLoad=(double)atoi(token);
				else if(i%3==2)
					Mapping[a.taskID]=atoi(token);
				token=strtok(NULL,",");
				i++;
			}
			Tasks.push_back(a);

		}		
	fclose(fp);


	//update Cores
	stringstream temp1;
		temp1<<time;
		string b="./TIME/t"+temp1.str()+"/coreLoads.txt";
		FILE * fo=fopen(b.c_str(),"r");
		for(int i=0;i<16;i++)
		{
			core c;
			c.id=i;
			fscanf(fo, "%lf",&c.coreLoad);
			Cores.push_back(c);
		}
		fclose(fo);


}

double calculateNUCOfactor(task t, core c)
{
	double ans=0;
	for(int i=0;i<Cores.size();i++)
	{
		ans+=latency[Cores[i].id][c.id];
	}
	return ans;
}



double calculateCost(task t,core c)
{
return (double)(c.coreLoad+FACTOR*(RCOMM*calculateNUCOfactor(t,c)-LCOMM));
}


int getCoreId(core Core)
{
	int j;
	for(j=0;j<Cores.size();j++)
		if(Cores[j].id==Core.id)
			break;
	return j;
}


bool comp(task t1,task t2)
{
	return t1.taskLoad > t2.taskLoad;

}


void balance_load()
{
	New_map=Mapping;
	sort(Tasks,Tasks+Tasks.size(),comp);
	for(int i=0;i<Tasks.size();i++)
	{
		core CoreDetails=Cores[Mapping[Tasks[i].taskID]];
		int k=getCoreId(CoreDetails);
		Cores[k].coreLoad-=Tasks[i].taskLoad;
		New_map[Tasks[i]]=NULL;
		double minCost=999999;
		core newCore;
		for(int j=0;j<Cores.size();j++)
			if(calculateCost(Tasks[i],Cores[j])<minCost){
				minCost=calculateCost(Tasks[i],Cores[j]);
				newCore=Cores[j];
			}
		k=getCoreId(newCore);
		Cores[k].coreLoad+=Tasks[i].taskLoad;
		New_map[Tasks[i].taskID]=newCore.id;
	}
}


int main(int argc,char *argv[])
{

	init();

int  numtasks, taskid, len;
char hostname[MPI_MAX_PROCESSOR_NAME];
int  partner, message;
MPI_Status status;

MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
MPI_Get_processor_name(hostname, &len);



while(time<MAX_ITERATIONS){
	time++;
	update();
	balance_load();
	map<task,core>::iterator it1=Mapping.begin();
	map<task,core>::iterator it2=New_map.begin();

	for(;it1!=Mapping.end(),it2!=New_map.end();it1++,it2++)
	{
		if(getCoreId(it2->second)==getCoreId(it1->second))
			continue;
		task t=it1->first;
		if(taskid==getCoreId(it1->second))
		{
			MPI_Send(&t,2,MPI_INT,it2->second,1,MPI_COMM_WORLD);//send task
		}
		else if(taskid==getCoreId(it2->second))
		{
			Task u;
			MPI_Recv(&u,2,MPI_INT,it1->second,1,MPI_COMM_WORLD,&status);//Receive task
			//System call to assign task to core
		}
	}

	if(taskid==0)
	{
		sleep(SLEEP_TIME);
		for(int i=1;i<Cores.size();i++)
			MPI_Send(1,1,MPI_INT,i,1,MPI_COMM_WORLD);
	}
	else
		MPI_Recv(1,1,MPI_INT,0,1,MPI_COMM_WORLD,&status);
	
}

MPI_Finalize();
return 0;
}
