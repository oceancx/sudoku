#include <iostream>
#include <string>
using namespace std;
#define NUM9 9
class Sudoku
{
public:
	Sudoku(int sudokuArray[81])
	{
		for (int i = 0; i < 81; i++) {
			int col = i % 9;
			int row = i / 9;
			mSudokuMat[row][col] = sudokuArray[i];
		}
		initMatSets();

	};

	~Sudoku()
	{

	};

	void initMatSets()
	{
		memset(mMatSets, 0, sizeof(mMatSets));
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < 9; j++)
			{
				if (mSudokuMat[i][j] == 0)
				{
					mMatSets[i][j] = (1 << 10) - 2;
				}
				else
				{
					mMatSets[i][j] = 0;
				}
			}
		}
		//memset(changeElement, false, sizeof(changeElement));
	};

	int& getCandidateSet(int row, int col)
	{
		return mMatSets[row][col];
	};

	//根据输入 求解这个数独
	/*
	求解算法：
	一遍又一遍的遍历整个9x9格子，
	1. 根据横纵行，以及3x3格子块的情况，确定当前格子可选数字 如1~9等
	如果当前格子可以确定，那就用当前格子的数字，去剔除掉能影响到的格子
	如果不可以确定，那么就把候选的数字集合保留起来
	2. 遍历一遍后，再接着遍历，返回1
	*/
	//初始化 让所有为0的格子的候选集合都为1~9

	//std::cout << "what the fuck" << getCandidateSet(3, 1)[0] << std::endl;


	void swapMat9x9(bool isBackup, int backup[9][9], int origin[9][9])
	{
		for (int i = 0; i < 81; i++)
		{
			int row = i / 9, col = i % 9;
			if (isBackup) {
				backup[row][col] = origin[row][col];
			}
			else {
				origin[row][col] = backup[row][col];
			}

		}
	}


	void backupSudokuMat(int dep)
	{
		if (changeElement[dep])return;
		swapMat9x9(true, mBackupSudokuSets[dep], mSudokuMat );
	}
	void restoreSudokuMat(int dep)
	{
		swapMat9x9(false, mBackupSudokuSets[dep], mSudokuMat);
	}


	void backupAllCandidate(int dep)
	{
		if (changeElement[dep])return;
		swapMat9x9(true, mBackupMatSets[dep], mMatSets);
	}
	void restoreAllCandidate(int dep)
	{
		swapMat9x9(false, mBackupMatSets[dep], mMatSets);

	}

	bool changeElement[82] = {};
	bool solve(int dep)
	{

		for (int x = 0; x < 100; x++)
		{
			bool changed = false;
			for (int i = 0; i < 9; i++)
			{
				for (int j = 0; j < 9; j++)
				{
					int& val = mSudokuMat[i][j];
					//此时val的值是不确定的   那就根据横纵行 还有小方块，来更新当前块的候选数字
					//如果更新成功 那么就把val改好
					if (val == 0)
					{
						int& candidateSet = getCandidateSet(i, j);
						if (candidateSet == 0)
						{
							printf("candidateSet==0这里出错了 row:%d col:%d！！！！！\n", i, j);
							printSelf();
							return false;
						}
						int res = updateCandidateSet(i, j);
						if (res != -1)
						{
							changed = true;
							val = res;
							candidateSet = 0;
							printf("val值更新成功！ %d row:%d col:%d\n", val, i, j);
							printSelf();
						}
						else if (candidateSet == 0)
						{
							return false;
						}

					}
				}
			}

			printf("扫描第%d次！！！\n", x);
			printPosibilities();
			printSelf();

			if (isSolved())
			{
				return true;
			}

			if (!changed)
			{
				// printPosibilities();
				//尝试优化
				for (int i = 0; i < 9; i++)
				{
					int srow = i / 3 * 3;
					int scol = i % 3 * 3;
					int oneCnt = 0;
					int filter[10] = {};
					int filterAttach[10] = {};
					for (int j = 0; j < 3; j++)
					{
						for (int k = 0; k < 3; k++)
						{
							int erow = srow + j;
							int ecol = scol + k;
							int& cset = getCandidateSet(erow, ecol);
							for (int p = 1; p <= 9; p++)
							{
								if ((cset >> p) & 1)
								{
									filter[p]++;
									filterAttach[p] = erow * 9 + ecol;
								}
							}
						}
					}

					for (int q = 1; q <= 9; q++)
					{
						if (filter[q] == 1)
						{
							int erow = filterAttach[q] / 9;
							int ecol = filterAttach[q] % 9;
							mMatSets[erow][ecol] = 0;
							mSudokuMat[erow][ecol] = q;
							changed = true;
						}
					}

				}
			}


			/*如果集合没有变化，那么就从canSet里面的元素，进行枚举，直到能推出整个*/
			if (!changed)
			{
				int minIndex = -1;
				int minCanCnt = 1000;
				for (int i = 0; i < 9; i++)
				{
					for (int j = 0; j < 9; j++)
					{
						int& canSet = getCandidateSet(i, j);
						if (canSet != 0)
						{
							int setCnt = getSetCnt(canSet);
							if (setCnt < minCanCnt)
							{
								minCanCnt = setCnt;
								minIndex = i * 9 + j;
							}
						}
					}
				}

				if (minIndex != -1)
				{
					int& minCanset = getCandidateSet(minIndex / 9, minIndex % 9);
					for (int k = 1; k <= 9; k++) {
						if ((minCanset >> k) & 1)
						{
							backupAllCandidate(dep);
							backupSudokuMat(dep);
							changeElement[dep] = true;
							mSudokuMat[minIndex/9][minIndex%9] = k;
							minCanset = 0;
							//minCanset &= ~(1 << k);
							if (solve(dep+1))
							{
								printf("更换元素 搜索成功\n");
								return true;
							}
							else
							{
								printf("dep:%d 更换元素 搜索失败！！！！！\n",dep);

								printSelf();
								restoreAllCandidate(dep);
								restoreSudokuMat(dep);
								changeElement[dep] = false;
								//printPosibilities();
								mSudokuMat[minIndex / 9][minIndex % 9] = 0;
								minCanset &= ~(1 << k);
							}
						}
					}
					return false;
				}
			}
		}
		return false;
	}

	void printPosibilities()
	{
		int cnt = 0;
		long long posibilities = 1;
		int undeterminedElements = 0;

		for (int i = 0; i < 9; i++)
		{
			int srow = i / 3 * 3;
			int scol = i % 3 * 3;
			printf("i:%d\n", i + 1);
			for (int j = 0; j < 3; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					int erow = srow + j;
					int ecol = scol + k;

					int& set = getCandidateSet(erow, ecol);
					int cnt = getSetCnt(set);
					if (cnt != 0)
					{
						undeterminedElements++;
						posibilities *= cnt;
						printf("(%d,%d,n=%d):", erow + 1, ecol + 1, cnt);
						printSet(set);
						printf("\t");
					}

				}
				printf("\n");
			}
		}

		printf("剩余可能的组合数:%lld  未确定元素数：%d\n", posibilities, undeterminedElements);

	}


	void printSet(int set)
	{
		int count = 0;
		for (int i = 1; i <= 9; i++)
		{
			if ((set >> i) & 1)
			{
				if (count == 0)
				{
					printf("{%d", i);
					count++;
				}
				else
				{
					printf(",%d", i);
				}
			}
		}
		printf("}");
	}


	int getSetCnt(int set)
	{
		int cnt = 0;
		for (int i = 1; i <= 9; i++)
		{
			if ((set >> i) & 1)
			{
				cnt++;
			}
		}
		// printf("Cnt:%d\n",cnt);
		return cnt;
	}

	int updateCandidateSet(int row, int col)
	{
		//求出横竖行还有小方格的并集 然后用val的候选集减去这个并集 便能成功更新这个候选集
		int& candidateSet = getCandidateSet(row, col);
		for (int i = 0; i < 9; i++)
		{
			if (mSudokuMat[row][i] != 0)
			{
				candidateSet &= ~(1 << mSudokuMat[row][i]);
			}

			if (mSudokuMat[i][col] != 0)
			{
				candidateSet &= ~(1 << mSudokuMat[i][col]);
			}
		}

		int smallSetStartRow = row / 3 * 3;
		int smallSetStartCol = col / 3 * 3;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				int& innerVal = mSudokuMat[smallSetStartRow + i][smallSetStartCol + j];
				if (innerVal != 0)
				{
					candidateSet &= ~(1 << innerVal);
				}
			}
		}

		if (candidateSet != 0)
		{
			int cnt = 0;
			int candidateNum = -1;
			for (int i = 1; i <= 9; i++)
			{
				if ((candidateSet >> i) & 1)
				{
					cnt++;
					candidateNum = i;
				}
			}

			if (cnt == 1)
			{
				return candidateNum;
			}
		}
		return -1;
	}

	/*
	判断一个数独是否正确
	*/
	bool isSolved()
	{
		/*
		判断每一行 是否OK
		*/
		for (int row = 0; row < 9; row++)
		{
			if (!isSetOk(mSudokuMat[row])) {
				return false;
			}
		}

		/*
		判断每一列 是否OK
		*/
		for (int col = 0; col < 9; col++)
		{
			int colSet[9];
			for (int i = 0; i < 9; i++)
			{
				colSet[i] = mSudokuMat[i][col];
			}
			if (!isSetOk(colSet)) {
				return false;
			}
		}

		/*
		判断每一个小集合是否OK
		*/
		for (int iset = 0; iset< 9; iset++)
		{
			int innerSet[9], outRow, outCol;

			outRow = iset / 3 * 3;
			outCol = iset % 3 * 3;

			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					innerSet[i * 3 + j] = mSudokuMat[outRow + i][outCol + j];
				}
			}
			//printf("输出InnerSet i:%d\n", iset);
			//print(3, innerSet);

			if (!isSetOk(innerSet)) {
				return false;
			}
		}
		printf("数独已完成\n");
		return true;
	}



	bool isSetOk(int(&set)[9])
	{
		int emptySet[10] ={};

		for (int i = 0; i < 9; i++) {
			if (set[i] == 0)return false;
			emptySet[set[i]]++;
			if (emptySet[set[i]] > 1) {
				printSelf();
				printf("数独仍待解决\n");
				return false;
			}
		}
		return true;
	}

	void print(int dimension, int* mat)
	{
		for (int i = 0; i < dimension; i++) {
			for (int j = 0; j < dimension; j++) {
				printf("%d ", mat[i*dimension + j]);
				if ((j + 1) % 3 == 0)printf(" ");
			}
			if ((i + 1) % 3 == 0)printf("\n");
			printf("\n");
		}
	}

	void printSelf()
	{
		printf("输出mSudokuMat:\n");
		print(9, &(mSudokuMat[0][0]));
	}
private:
	int mSudokuMat[9][9];
	int mMatSets[9][9];
	int mBackupSudokuSets[100][9][9] = {};
	int mBackupMatSets[100][9][9] = {};
};


int InputSudoku[] = {


	//0,0,0, 0,0,0, 0,0,0,
	8,0,0, 0,0,0, 0,0,0,
	0,0,3, 6,0,0, 0,0,0,
	0,7,0, 0,9,0, 2,0,0,

	0,5,0, 0,0,7, 0,0,0,
	0,0,0, 0,4,5, 7,0,0,
	0,0,0, 1,0,0, 0,3,0,

	0,0,1, 0,0,0, 0,6,8,
	0,0,8, 5,0,0, 0,1,0,
	0,9,0, 0,0,0, 4,0,0,

	/*0,5,0, 0,0,1, 0,0,0,
	0,0,0, 0,0,0, 0,0,0,
	0,0,4, 0,7,0, 0,0,9,

	0,0,0, 0,0,2, 1,5,0,
	7,0,9, 0,8,0, 0,0,0,
	0,0,0, 0,0,0, 0,3,0,

	0,0,7, 9,0,0, 0,0,0,
	0,0,0, 0,0,0, 0,1,0,
	0,0,8, 0,6,0, 0,0,0*/


	/*0,0,0, 0,0,0, 0,0,0,
	0,9,7, 0,8,0, 0,0,0,
	0,0,0, 3,0,0, 0,0,1,

	0,0,0, 0,0,0, 9,7,0,
	3,0,0, 1,0,0, 0,2,0,
	0,0,0, 0,6,0, 8,0,0,

	6,0,0, 0,0,0, 0,0,0,
	1,0,0, 0,0,5, 0,0,4,
	0,0,0, 0,9,0, 0,0,0,*/


	/*3,0,2, 0,0,4, 0,0,8,
	7,0,0, 0,1,0, 0,0,9,
	0,0,0, 0,2,0, 7,0,6,

	0,7,0, 1,3,0, 0,0,4,
	0,3,0, 0,9,8, 0,0,0,
	1,0,0, 0,0,0, 0,0,0,

	0,0,0, 3,4,9, 0,8,0,
	0,0,3, 0,0,0, 6,0,0,
	0,0,0, 0,7,6, 0,9,0,*/



	/*0,0,0, 8,6,5, 9,0,0,
	7,0,0, 0,0,0, 6,0,1,
	0,2,6, 0,0,0, 0,0,0,

	9,0,0, 0,7,0, 0,0,5,
	0,6,0, 3,0,0, 2,0,0,
	0,0,0, 0,0,4, 0,0,0,

	6,0,0, 9,1,3, 0,0,0,
	0,0,0, 0,0,0, 0,1,6,
	3,8,0, 0,0,0, 4,0,0,*/



	/*0,9,0, 0,2,8, 7,0,0,
	7,0,0, 0,0,0, 0,0,3,
	0,0,2, 0,3,0, 0,0,0,

	7,4,0, 0,0,0, 0,6,0,
	0,0,0, 0,9,0, 8,0,0,
	0,0,3, 4,0,0, 0,0,0,

	1,0,0, 0,0,0, 0,9,5,
	0,6,0, 0,1,9, 3,8,0,
	9,0,0, 3,7,0, 0,0,0,
	*/

	// 0,0,0,0,5,4,9,0,0,
	// 1,6,0,0,0,0,0,0,0,
	// 2,0,0,0,0,0,0,0,0,
	// 0,0,8,0,0,0,7,0,0,
	// 0,0,0,0,0,0,0,0,0,
	// 0,0,0,6,0,1,0,0,2,
	// 0,0,7,0,9,0,8,0,0,
	// 0,0,0,0,7,0,0,0,0,
	// 3,0,0,0,0,0,0,7,1

	// 8,7,3,2,5,4,9,1,6,
	// 1,6,9,8,3,0,5,4,7,
	// 2,5,4,9,1,6,3,8,0,

	// 9,4,8,5,2,3,7,6,0,
	// 6,2,0,0,4,7,1,0,8,
	// 7,3,5,6,8,1,4,9,2,

	// 0,1,7,3,9,2,8,5,4,
	// 4,8,2,1,7,5,6,3,9,
	// 3,9,0,4,6,8,2,7,1

	/* 3,0,2, 0,0,8 ,0,0,7,
	0,0,0, 0,0,0 ,0,0,0,
	4,0,0, 5,9,0 ,0,6,0,
	0,0,4, 0,0,9 ,0,0,3,
	6,0,8, 0,4,0 ,0,2,0,
	2,0,1, 6,7,5 ,0,9,0,
	0,0,0, 0,0,0 ,0,0,0,
	5,7,0, 1,0,3 ,0,0,4,
	0,0,0, 8,2,7 ,0,0,0*/

	/* 0,8,7, 0,0,5, 0,3,0,
	0,0,0, 7,0,0, 0,2,0,
	0,0,0, 0,4,0, 5,9,0,
	0,0,0, 0,0,3, 0,4,0,
	6,0,0, 0,9,0, 0,0,0,
	0,3,0, 0,5,0, 0,0,0,
	0,5,1, 9,0,0, 0,0,0,
	4,0,0, 0,0,1, 8,0,0,
	0,0,6, 0,3,0, 1,0,0,*/



	/* 0,0,0 ,0,2,4 ,0,6,9,
	9,0,0 ,1,0,0 ,7,8,0,
	8,5,0 ,0,9,0 ,0,3,0,
	0,1,0 ,0,0,3 ,0,9,6,
	5,2,0 ,0,8,0 ,0,0,0,
	0,0,7 ,9,0,2 ,0,0,4,
	0,8,1 ,0,4,0 ,0,0,5,
	3,6,4 ,0,0,1 ,0,0,8,
	0,0,5 ,7,0,8 ,1,0,0*/


	//	3,9,6,5,8,4,1,7,2,
	//2,7,5,1,3,9,6,8,4,
	//1,8,4,7,2,6,5,3,9,
	//6,0,8,9,4,5,0,1,7,
	//9,0,1,0,6,7,8,0,5,
	//5,0,7,8,1,0,9,0,6,
	//7,1,0,6,9,8,4,0,3,
	//4,5,9,0,7,1,0,6,8,
	//8,6,0,4,5,0,7,9,1

	/*0,7,2,9,0,0,0,1,0,
	0,0,0,0,3,0,0,9,8,
	8,0,5,4,7,0,0,0,0,
	6,1,4,0,0,0,0,2,3,
	0,0,0,0,2,3,4,0,1,
	7,0,0,1,9,0,0,6,0,
	0,3,7,0,0,2,0,0,0,
	4,0,8,0,0,0,2,5,0,
	0,5,0,0,8,6,3,0,9*/



};

int OkSudoku[] = {
	3,9,6,5,8,4,1,7,2,
	2,7,5,1,3,9,6,8,4,
	1,8,4,7,2,6,5,3,9,
	6,2,8,9,4,5,3,1,7,
	9,3,1,2,6,7,8,4,5,
	5,4,7,8,1,3,9,2,6,
	7,1,2,6,9,8,4,5,3,
	4,5,9,3,7,1,2,6,8,
	8,6,3,4,5,2,7,9,1
};




int main()
{
	Sudoku mSudo(InputSudoku);
	mSudo.printSelf();
	mSudo.solve(0);
	//mSudo.isSolved();
	while (true);
	return 0;
}


// 0,0,0,0,0,0,0,0,0,
// 	0,0,0,0,0,0,0,0,0,
// 	0,0,0,0,0,0,0,0,0,
// 	0,0,0,0,0,0,0,0,0,
// 	0,0,0,0,0,0,0,0,0,
// 	0,0,0,0,0,0,0,0,0





