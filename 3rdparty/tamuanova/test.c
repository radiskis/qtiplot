//#include <config.h>
//#include <stdlib.h>
#include <gsl/gsl_test.h>
#include <gsl/gsl_ieee_utils.h>
#include "tamu_anova.h"
//#include <malloc.h>

/*begin declarations*/
double * data;
long * factor[2];

int oneway_balanced_test();
int twoway_balanced_test();
int twoway_unbalanced_test_1();
int oneway_unbalanced_test();

int compare_twoway_tables(const struct tamu_anova_table_twoway t1,const struct tamu_anova_table_twoway t2, const double tol);
/*end declarations*/


int main()
{
	gsl_ieee_env_setup ();

	oneway_balanced_test();
	oneway_unbalanced_test();

	twoway_balanced_test();
	twoway_unbalanced_test_1();

	return gsl_test_summary();
};

int twoway_unbalanced_test_1()
{
/*Test data from Searle*/
double d[15]={
6,10,11,13,15,14,22,12,15,19,18,31,18,9,12};
long f[15][2]={
{1,1},
{1,1},
{1,1},
{1,2},
{1,2},
{1,3},{1,3},
{2,1},{2,1},{2,1},{2,1}
,{2,2},
{2,3},{2,3},{2,3}};

long J[2]={2,3};
struct tamu_anova_table_twoway R = tamu_anova_twoway(d,f,15,J,anova_fixed);

struct tamu_anova_table_twoway r;
r.SSA = 123.0+27.0/35.0;r.dfA=1;r.MSA=r.SSA;r.FA=9.282857;r.pA = 0.01386499;
r.SSB = 192.1276596;r.dfB=2;r.MSB=96.0638298;r.FB=7.204787;r.pB = 0.01354629;
r.SSAB = 222.7659574;r.dfAB = 2; r.MSAB = 111.3829787;r.FAB = 8.353723;r.pAB = 0.00888845;
r.SSE = 120.0; r.dfE=9; r.MSE = 120.0/9.0;
r.SST = 520.0; r.dfT=14;

int status = compare_twoway_tables(r,R,0.000001);
gsl_test(status,"Unbalanced twoway fixed ANOVA test with data from Searle:%d",status);
return status;
};



int compare_twoway_tables(const struct tamu_anova_table_twoway t1,
const struct tamu_anova_table_twoway t2, const double tol){
int sum=0;
sum += (t1.SSA 		- t2.SSA	>	tol);//printf("%d",sum);
sum += (t1.SSB 		- t2.SSB	> tol);	//printf("%d",sum);
sum += (t1.SSAB 	- t2.SSAB> tol);//printf("%d",sum);
sum += (t1.SSE 	- t2.SSE> tol);//printf("%d",sum);
sum += (t1.SST 	- t2.SST> tol);//printf("%d",sum);
sum += (t1.dfA 	!= t2.dfA);		//printf("%d",sum);
sum += (t1.dfB 	!= t2.dfB);	//printf("%d",sum);
sum += (t1.dfAB	!= t2.dfAB);//printf("%d",sum);
sum += (t1.dfE	!= t2.dfE);//printf("%d",sum);
sum += (t1.dfT	!= t2.dfT);//printf("%d",sum);
sum += (t1.MSA 		- t2.MSA 	> tol);//printf("%d",sum);
sum += (t1.MSB 		- t2.MSB 	> tol);//printf("%d",sum);
sum += (t1.MSAB 	- t2.MSAB> tol);//printf("%d",sum);
sum += (t1.MSE 	- t2.MSE> tol);//printf("%d",sum);
sum += (t1.FA 		- t2.FA > tol);//printf("%d",sum);
sum += (t1.FB 		- t2.FB > tol);//printf("%d",sum);
sum += (t1.FAB 	- t2.FAB > tol);//printf("%d",sum);
sum += (t1.pA 		- t2.pA > tol);//printf("%d",sum);
sum += (t1.pB 		- t2.pB > tol);//printf("%d",sum);
sum += (t1.pAB 	- t2.pAB > tol);//printf("%d",sum);
return sum;
};

int twoway_balanced_test(){
double data[36]={
10.5, 9.2,  7.9,  8.1,  8.6,  10.1, 16.1, 15.3, 17.5,
12.8, 11.2, 13.3, 12.7, 13.7, 11.5, 16.6, 19.2, 18.5,
12.1, 12.6, 14.0, 14.4, 15.4, 13.7, 20.8, 18.0, 21.0,
10.8, 9.1 , 12.5, 11.3, 12.5, 14.5, 18.4, 18.9, 17.2};

long factor[36][2]={
{1,1},{1,1},{1,1},{2,1},{2,1},{2,1},{3,1},{3,1},{3,1},
{1,2},{1,2},{1,2},{2,2},{2,2},{2,2},{3,2},{3,2},{3,2},
{1,3},{1,3},{1,3},{2,3},{2,3},{2,3},{3,3},{3,3},{3,3},
{1,4},{1,4},{1,4},{2,4},{2,4},{2,4},{3,4},{3,4},{3,4}};

long J[2]={3,4};
struct tamu_anova_table_twoway R = tamu_anova_twoway(data,factor,36,J,anova_fixed);

struct tamu_anova_table_twoway r;
r.SSA = 327.5972222;r.dfA=2;r.MSA=163.798611;r.FA=103.3430;r.pA =0 ;
r.SSB = 86.6866667;r.dfB=3;r.MSB=28.8955556;r.FB=18.23063;r.pB =0.000002212 ;
r.SSAB = 8.031667;r.dfAB = 6; r.MSAB = 1.3386111;r.FAB =0.8445496 ;r.pAB =0.5483607 ;
r.SSE = 38.04;r.dfE=24;r.MSE = 1.585;
r.SST = 460.3555556;r.dfT=35;
//r.FA=;r.pA = ;r.FB=;r.pB = ;r.FAB = ;r.pAB = ;

int status = compare_twoway_tables(r,R,0.0001);
gsl_test(status,"Balanced twoway fixed ANOVA test with data from Devore:%d",status);
return status;
};

int compare_tables(const struct tamu_anova_table t1,
const struct tamu_anova_table t2, const double tol){
int sum=0;
sum += (t1.SSTr   - t2.SSTr > tol);
sum += (t1.SSE  - t2.SSE> tol);
sum += (t1.SST  - t2.SST> tol);
sum += (t1.dfTr   != t2.dfTr);
sum += (t1.dfE != t2.dfE);
sum += (t1.dfT != t2.dfT);
sum += (t1.MSTr    - t2.MSTr > tol);
sum += (t1.MSE  - t2.MSE> tol);
sum += (t1.F     - t2.F > tol);
sum += (t1.p     - t2.p > tol);
return sum;
};


int oneway_balanced_test(){
double data[20] = {
88.60,73.20,91.40,68.00,75.20,63.00,53.90,
69.20,50.10,71.50,44.90,59.50,40.20,56.30,
38.70,31.00,39.60,45.30,25.20,22.70,
};
long factor[20]={
1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,4,4,4,4,4
};
struct tamu_anova_table R = tamu_anova(data,factor,20,4);

struct tamu_anova_table r;
r.dfTr = 3;r.SSTr = 5882.3575;r.MSTr = 1960.785833;
r.dfE = 16;r.SSE = 1487.4;r.MSE=92.9625;
r.dfT = 19;r.SST = 7369.7575;
r.F = 21.09222356326;
r.p = .000008324882;

int status = compare_tables(r,R,0.0000001);
gsl_test(status,"Balanced oneway ANOVA test with data from Devore:%d",status);
return status;
};

int oneway_unbalanced_test(){
double data[22] = {
45.50,45.30,45.40,44.40,44.60,43.90,44.60,44.00,44.20,43.90,44.70,
44.20,44.00,43.80,44.60,43.10,46.00,45.90,44.80,46.20,45.10,45.50
};
long factor[22]={
1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3
};
struct tamu_anova_table R = tamu_anova(data,factor,22,3);

struct tamu_anova_table r;
r.dfTr = 2;r.SSTr = 7.93007576;r.MSTr = 3.96503788;
r.dfE = 19;r.SSE = 5.99583333;r.MSE=0.31557018;
r.dfT = 21;r.SST = 13.92590909;
r.F = 12.564678576;
r.p = 0.0003336161;

int status = compare_tables(r,R,0.0000001);
gsl_test(status,"Unbalanced oneway ANOVA test with data from Devore:%d",status);
return status;
};

