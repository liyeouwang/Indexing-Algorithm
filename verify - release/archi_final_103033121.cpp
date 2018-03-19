#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <bitset>
using namespace std;

double countQ(char *ptr,int addressing_Bus, int count, int bit);
double countC(char *ptr, int addressing_Bus, int count, int bit1, int bit2);
void IndexOrder(int addressing_Bus, int* AbestIntPtr, double* Qptr,double *Cptr);
int main(int argc, char* argv[])
{
//read cache file
	ifstream cache(argv[1]); 
	string s[9];
	for(int i = 0; i<9;i++)
		cache>>s[i]; 
	int addressing_Bus = atoi( s[2].c_str() ) ;
	int entries = atoi(s[5].c_str());
	int associativity = atoi(s[8].c_str());
	int seletBitNum = log2(entries);

//read data file	
	ifstream ref(argv[2]); 
	string line,line2; 
	string x;	
	int dataCount =-1;
   	while(getline(ref,line))	//counting the amount of the data
   	{
   		istringstream in( line );
   		in>>x;
   		if(x==".end")
   			break;
   		dataCount++;
   	}

   	char array[dataCount][addressing_Bus];  //creating an array to store the data
   	
   	ifstream ref2(argv[2]); 
   	string firstLine;
   	getline(ref2,firstLine);
   	for(int i =0; i<dataCount ; i++)	//store the data one by one into the array
   	{
   		string temp;
   		getline(ref2,line2);
   		istringstream in2( line2 );
   		in2>>temp;
   		for(int j=0;j<addressing_Bus;j++) 
   			array[i][j]=temp[j];	
   	}

//counting the quality measures
	double Q[addressing_Bus];
	for(int i=0;i<addressing_Bus;i++)
	{
		Q[i]=countQ(&array[0][0],addressing_Bus,dataCount,i);
	} 
	
//counting the correlation measures
	double C[addressing_Bus][addressing_Bus];
	for(int i=0;i<addressing_Bus;i++)
	{
		for(int j=0;j<addressing_Bus;j++)
		{
			C[i][j]=countC(&array[0][0],addressing_Bus,dataCount,i,j);
		}
	}

//call the argorithm to caculate the best index bit order
	int AbestInt[addressing_Bus];
	IndexOrder(addressing_Bus, AbestInt, Q ,&C[0][0]);

// pick the indexing bits and sort them in ascending  order
	sort(AbestInt, AbestInt + seletBitNum);
	
//	start to check each data, determine hit or miss
	int count[entries][associativity];	//when replacing, replace the one with the largest count number
	for(int i=0;i<entries;i++)
	{
		for(int j=0;j<associativity;j++)
		{ 
			count[i][j] = 0;
		} 
	}
	string block[entries][associativity];
	string result[dataCount][2];	//store to output in the file("data miss" or "data hit")
	int index[seletBitNum];
	int missCount = 0 ;
	for(int i=0;i<dataCount;i++)
	{
		int tag=0;
		
		for(int j=0;j<seletBitNum;j++)	//select the index bits
		{
			index[j]= array[i][AbestInt[j]]-'0';
		}
		for(int j=seletBitNum-1;j>=0;j--)	//counvert tag number from binary to decimal
		{
			for(int k =0;k<j;k++) index[seletBitNum-1-j]*=2;
		}
		for(int j=0;j<seletBitNum;j++)	//determin the tag number(decimal form)
		{
			tag+=index[j];
		}
		string data(array[i],addressing_Bus);	//storing the data form array format to string format	

		//placing data into the block
		//update the  count number
		bool dataIn = false;
		for(int j=0;j<associativity;j++) //check hit or miss
		{			
			if(block[tag][j] ==data)	//data in the block
			{
				dataIn = true;
				result[i][0]= data;
				result[i][1]= "hit";
				count[tag][j] = 1;
				for(int k=0;k<associativity;k++)
					if(count[tag][k]) count[tag][k]++;
				break;
				//change the count to 1
				//all the block(with same tag) count ++
			}
			else if(count[tag][j] == 0)  //empty block
			{
				dataIn = true;
				result[i][0]= data;
				result[i][1]= "miss";
				missCount++;
				block[tag][j] = data;
				count[tag][j] = 1;
				for(int k=0;k<associativity;k++)
					if(count[tag][k]) count[tag][k]++;
				break;
				//change the count to 1
				//all the block(with same tag) count ++
			}						
		}
		if(!dataIn)	//	implement LRU policy, replace the one with the largest count number
		{
			result[i][0]= data;
			result[i][1]= "miss";
			missCount++;
			int countMax = count[tag][0];
			int MaxNum = 0; 
			for(int j=0;j<associativity;j++)
			{
				if(count[tag][j]>countMax)
				{
					countMax = count[tag][j];
					MaxNum = j;
				}
			}
			block[tag][MaxNum] = data;
			count[tag][MaxNum] = 1;
			for(int j=0;j<associativity;j++)
				count[tag][j]++;
			//change the count to 1
			//all the block(with same tag) count ++
		}		
	}

//output
	ofstream output;
	output.open(argv[3]);

	output<<"Student ID: 103033121"<<endl;
	output<<"Addressing Bus: "<<addressing_Bus<<endl;
	output<<"Entries: "<<entries<<endl;
	output<<"Associativity: "<<associativity<<endl;
	output<<"Indexing bits count: "<<seletBitNum<<endl;
	output<<"Indexing bits: ";
	for(int i=0;i<seletBitNum;i++)
		output<<AbestInt[i]<<" ";
	output<<endl;
	output<<"Total cache miss: "<<missCount<<endl;
	output<<endl;
	output<<firstLine<<endl;
	for(int i=0;i<dataCount;i++)
		output<<result[i][0]<<" "<<result[i][1]<<endl;
	output<<".end"<<endl;
  	output.close();
	
   return 0;
}
///functions///////////////////////////////////////////////////
double countQ(char *ptr, int addressing_Bus, int count, int bit)
{
	int Z=0; 
	int O=0;
	for(int i=0;i<count;i++)
	{
		if(*(ptr + i*addressing_Bus + bit) == '0') Z++;
		else O++;
	}
	return double(min(Z,O))/double(max(Z,O));
}
///functions///////////////////////////////////////////////////
double countC(char *ptr, int addressing_Bus, int count, int bit1, int bit2)
{
	int E=0;
	int D=0;
	for(int i=0;i<count;i++)
	{
		if(*(ptr + i*addressing_Bus + bit1) == *(ptr + i*addressing_Bus + bit2)) E++;
		else D++;
	}
	return double(min(E,D))/double(max(E,D));
}
///functions///////////////////////////////////////////////////
void IndexOrder(int addressing_Bus, int* AbestIntPtr, double* Qptr,double* Cptr)
{
	bool AExist[addressing_Bus];
	for(int i=0;i<addressing_Bus;i++)
		AExist[i] = false;
	for(int i =0;i<addressing_Bus;i++)
	{
		double maxQ = -1;
		int bestNum;		
		for(int j = 0; j<addressing_Bus; j++)  //find max(Q1...QM-1), determin Abest 
	    {
	        if(*(Qptr+j) > maxQ && !AExist[j])
	        {
				maxQ = *(Qptr+j);
	           	bestNum = j;	
	        }
	    }
	    AExist[bestNum] = true; 
	    *(AbestIntPtr+i) = bestNum;
	    for(int j=0;j<addressing_Bus;j++)
	    	*(Qptr+j) = (*(Qptr+j))*(*(Cptr + bestNum*addressing_Bus + j));	
	}
}
