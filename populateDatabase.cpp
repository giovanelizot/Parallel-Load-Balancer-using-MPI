#include<iostream>
#include<vector>
#include<fstream>
#include<map>
#include<string>
#include<stdlib.h>
#include<stdio.h>
#include<cstring>
#include<sstream>
using namespace std;


vector< vector<int> > mapping(4);
map<int,double> taskLoad;



int main()
{
	int time=1;
	for(time=1;time<=5;time++)
	{
		stringstream temp;
		temp<<time;
		string a="./TIME/t"+temp.str()+"/taskLoads.txt";
		FILE * fp=fopen(a.c_str(),"w+");
		
		
		int numTasks=rand()%10;
		for(int i=0;i<numTasks;i++){
			int taskId=rand()%1000;
			int coreNum=rand()%4;
			double load=rand()%100;
			taskLoad[taskId]=load;
			mapping[coreNum].push_back(taskId);
			fprintf(fp, "%d,%lf,%d\n",taskId,load,coreNum);
		}
		
		fclose(fp);
		
		stringstream temp1;
		temp1<<time;
		string b="./TIME/t"+temp1.str()+"/coreLoads.txt";
		FILE * fo=fopen(b.c_str(),"w+");
		for(int i=0;i<4;i++)
		{
			double totalCoreLoad=0;
			for(int j=0;j<mapping[i].size();j++)
			{
				totalCoreLoad+=taskLoad[mapping[i][j]];
			}
			fprintf(fo, "%lf\n",totalCoreLoad);
		}
		fclose(fo);

		stringstream temp2;
		temp2<<time;
		string c="./TIME/t"+temp2.str()+"/coreTaskMapping.txt";
		FILE * fq=fopen(c.c_str(),"w+");
		for(int i=0;i<4;i++)
		{
			for(int j=0;j<mapping[i].size();j++)
				fprintf(fq,"%d,",mapping[i][j]);\
			fprintf(fq, "\n");
		}
		fclose(fq);
	
	}
	return 0;
}