#include <fstream>
#include <ctime>
#include "graphics.h"
#pragma comment(lib, "graphics.lib")
#pragma warning(disable : 4996)

const int c = 40; //Kích thước ô, điểm bắt đầu(c, c)
#define round(a) int(a + 0.5)

struct Point
{
	int x, y;
};

struct ViTri
{
	int mau = 0;
	Point A;
	Point O;
};

ViTri vt[9][9]; //Bàn cờ

Point next[3]; //Hàng chờ chế độ trung bình, lưu tâm để vẽ

int tami, tamj; //Vị trí đang chọn

int xmin, ymin, xmax, ymax; //Giới hạn bàn cờ

int n; //Tổng số bi

int score; //Điểm

int bestscore; //Điểm cao nhất

int diQua[9][9] = { 0 }; //Xét đường đi ngắn nhất

//Xét 4 ô xung quanh
int ki[4] = { 0, 1, 0, -1 };
int kj[4] = { 1, 0, -1, 0 };

//Số bước ít nhất
int minStep;

int mode = 0; //0 = Chưa chọn, 1 = Dễ, 2 = Trung bình, 3 = Khó

void lineDDA(int, int, int, int, int);

void veHinhTron(Point, int, int);

void veHV(Point, int);

void randomNext();

bool showNext();

void waitClick(int&, int&);

void taoBanChoi();

void play(int&, int&);

bool xet(int, int);

void canLink(int, int, int, int, int);

void veHCN(int, int, int, int);

bool playAgain();

void showScore();

void showBestScore();

void saveBestScore();

void showHome();

int main()
{
	int gd = DETECT, gm;
	initgraph(&gd, &gm, "C:\\tc\\bgi");
	
	while(mode != -1)
	{
		showHome();
		if (mode != -1) //đã chọn mode mới tạo bàn chơi, -1 khi nhấn exit để thoát game luôn
		{
			do {
				taoBanChoi();
				if (mode == 0) //mode = 0 do người dùng nhấn home -> k cần hỏi play again
					break;
			} while (playAgain());
		}
	};

	//getch();
	closegraph();
	return 0;
}

void lineDDA(int x1, int y1, int x2, int y2, int color)
{
	int dx = x2 - x1;
	int dy = y2 - y1;

	int step = max(abs(dx), abs(dy));
	float x_inc = dx * 1.0 / step;
	float y_inc = dy * 1.0 / step;

	float x = x1, y = y1;
	putpixel(x, y, color);

	for (int i = 0; i < step; i++)
	{
		x += x_inc;
		y += y_inc;
		putpixel(round(x), round(y), color);
	}

}

void veHinhTron(Point o, int r, int color)
{
	//Trường hợp vẽ bi to vào vị trí đã có bi nhỏ cùng màu floodfill không tô được => vẽ bi đen đè lên trước
	if (color != 0)
		veHinhTron(o, r, 0);
	setcolor(color);
	circle(o.x, o.y, r); //vẽ trước vòng tròn để đánh dấu vùng tô
	setfillstyle(1, color); //1 -> tô kín
	floodfill(o.x, o.y, color);
}

void veHV(Point A, int color)
{
	int x = A.x;
	int y = A.y;
	for (int i = 1; i < 3; i++)
	{
		lineDDA(x + i, y + i, x + c - i, y + i, color);
		lineDDA(x + c - i, y + i, x + c - i, y + c - i, color);
		lineDDA(x + c - i, y + c - i, x + i, y + c - i, color);
		lineDDA(x + i, y + c - i, x + i, y + i, color);
	}
}

void randomNext()
{
	for (int k = 1; k < 4; k++)
	{
		int i = rand() % 9; //0 -> 8
		int j = rand() % 9;
		if (vt[i][j].mau == 0)
		{
			vt[i][j].mau = -((rand() % 7) + 8); //Sắp xuất hiện -> màu < 0 (màu từ 8 -> 14)
			if (mode == 1)
				veHinhTron(vt[i][j].O, c / 4, -vt[i][j].mau);
			if(mode == 2)
				veHinhTron(next[k - 1], c / 2 - 4, -vt[i][j].mau);
			n++;
		}
		else if (n < 81)
			k--;
	}
}

bool showNext()
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
		{
			if (vt[i][j].mau < 0)
			{
				veHinhTron(vt[i][j].O, c / 4, 0);
				vt[i][j].mau *= -1; //Chuyển giá trị màu thành dương
				veHinhTron(vt[i][j].O, c / 2 - 4, vt[i][j].mau);
				xet(i, j);
			}
		}
	randomNext();
	if (n >= 81)
		return false;
	return true;
}

void waitClick(int &i, int &j)
{
	int x, y;
	while (true) //Lặp lại đến khi đúng điều kiện để return
	{
		while (!ismouseclick(WM_LBUTTONDOWN)) //Chờ đến khi người dùng click chuột trái
		{
			delay(0);
		}
		getmouseclick(WM_LBUTTONDOWN, x, y); //Lấy vị trí mới nhất
		if (x > 475 && x < 575 && y > 225 && y < 255)
		{
			i = -1;
			return;
		}
		if (x > 500 && x < 550 && y > 300 && y < 350) //Nút back
		{
			mode = 0;
			return;
		}
		if (x > xmin && x < xmax && y > ymin && y < ymax) //Nếu vị trí không nằm trong bàn cờ -> chờ tiếp
		{
			j = (x - c) / c; //Nếu nằm trong bàn cờ chuyển từ vị trí sang số ô trong mảng
			i = (y - c) / c;
			return;
		}
	}
}

void taoBanChoi()
{
	int i, j;
	do
	{
		cleardevice();
		n = score = 0;
		xmin = ymin = c;
		xmax = ymax = 10 * c;
		tami = tamj = -1;
		if (mode == 2)
		{
			lineDDA(465, c, 465 + 3 * c, c, 15);
			lineDDA(465, 2 * c, 465 + 3 * c, 2 * c, 15);
			for (int i = 0; i < 4; i++)
			{
				lineDDA(465 + i * c, c, 465 + i * c, 2 * c, 15);
			}
		}

		//Ô điểm
		showScore();
		outtextxy(475, 180, "Score");

		outtextxy(475, 100, "Best score");
		showBestScore();

		//Nút new game
		veHCN(475, 225, 100, 30);
		outtextxy(487, 232, "New game");

		//Nút back
		veHCN(500, 300, 50, 50);
		outtextxy(505, 315, "Home");

		for (int i = 1; i <= 10; i++)
		{
			line(i * c, c, i * c, 10 * c);
			line(c, i * c, 10 * c, i * c);
		}

		for (int i = 0; i < 9; i++)
			for (int j = 0; j < 9; j++)
			{
				vt[i][j].A.x = (1 + j) * c;
				vt[i][j].A.y = (1 + i) * c;
				vt[i][j].O.x = vt[i][j].A.x + c / 2;
				vt[i][j].O.y = vt[i][j].A.y + c / 2;
				vt[i][j].mau = 0;
			}
		while (n < 3)
		{
			int i = rand() % 9;
			int j = rand() % 9;
			if (vt[i][j].mau == 0)
			{
				vt[i][j].mau = (rand() % 7) + 8;
				veHinhTron(vt[i][j].O, c / 2 - 4, vt[i][j].mau);
				n++;
			}
		}
		randomNext();
		play(i, j);
		if (mode == 0)
			return;
	} while (i == -1);
}

void play(int &i, int &j)
{
	while (true) {
		waitClick(i, j);
		if (i == -1 || mode == 0) //Back hoặc new game
			return;
		if (tami != -1) //Nếu có ô đang chọn trước thì bôi ô đó
			veHV(vt[tami][tamj].A, 0);
		veHV(vt[i][j].A, 15);
		tami = i;
		tamj = j;
		while (vt[tami][tamj].mau > 0) //Nếu ô đang chọn là ô có bi -> chờ ô tiếp theo
		{
			waitClick(i, j);
			if (i == -1 || mode == 0)
				return;
			else
			{
				if (vt[i][j].mau > 0) //Nếu ô chọn sau cũng có bi -> đổi ô chọn
				{
					veHV(vt[tami][tamj].A, 0);
					veHV(vt[i][j].A, 15);
					tami = i;
					tamj = j;
				}
				else //Nếu là ô trống, hoặc ô sắp xuất hiện -> di chuyển bi hiện tại đè lên nếu giữa 2 ô có đường đi
				{
					for each (int dq in diQua)
					{
						dq = 0; //Khởi tạo lại các bước đều là 0
					} 
					minStep = INT_MAX; //Khi tìm được đường đi lần đầu, số bước luôn được lưu vào
					canLink(tami, tamj, i, j, 0);
					Point p[81];
					if (minStep != INT_MAX) //Có đường đi
					{
						p[0].x = i; //Điểm đầu là đích
						p[0].y = j;
						p[minStep].x = tami; //Điểm cuối là ô hiện tại
						p[minStep].y = tamj;
						for (int s = 1; s < minStep; s++)
						{
							for (int k = 0; k < 4; k++) //Xét 4 ô xung quanh, tìm ô liền trước của ô hiện tại
							{
								int ii = p[s - 1].x + ki[k];
								int jj = p[s - 1].y + kj[k];
								if (ii >= 0 && ii <= 8 && jj >= 0 && jj <= 8 && diQua[ii][jj] == minStep - s) //Tìm ô có số đếm liền trước ô hiện tại
								{
									p[s].x = ii;
									p[s].y = jj;
									break;
								}
							}
						}
						veHV(vt[tami][tamj].A, 0); //Xóa chọn ô hiện tại
						int color = vt[tami][tamj].mau; //Lưu lại màu của bi
						vt[tami][tamj].mau = 0;
						int dem = minStep; //Đếm ngược từ số bước ít nhất về
						while (dem != 0)
						{
							veHinhTron(vt[tami][tamj].O, c / 2 - 4, 0); //Xóa bi ở ô trước
							if (vt[tami][tamj].mau < 0 && mode == 1)
								veHinhTron(vt[tami][tamj].O, c / 4, -vt[tami][tamj].mau); //Nếu ô đó là bi nhỏ và chế độ dễ thì sau khi đi qua phải vẽ lại
							dem--;
							tami = p[dem].x;
							tamj = p[dem].y;
							veHinhTron(vt[tami][tamj].O, c / 2 - 4, color); //Vẽ bi ô tiếp theo
							delay(100);
						}
						if (vt[i][j].mau < 0) //Ô sắp xuất hiện, giảm tổng số bi đi 1
							n--;
						vt[i][j].mau = color; //Set màu cho ô đích
						tami = -1;
						if (!xet(i, j) && !showNext()) //Nếu xét ăn được bi -> không cần hiển thị bi tiếp theo, nếu không ăn cũng không hiện được bi tiếp -> thua
							return;
						break; //Không break -> quay lại while xét màu, lúc này tami = -1 -> xét màu bị tràn -> lỗi
					}
				}
			}
		}
	}
}

bool xet(int i, int j)
{
	int color = vt[i][j].mau;
	int tam = 0; //lưu số bi đã đếm
	int dem = 0; //đếm bi mỗi hướng
	int giam, tang;
	int giami, giamj, tangi, tangj;

	//Xet hang doc
	for (giam = i - 1; giam >= 0; giam--) //Từ ô đó lùi về
	{
		if (vt[giam][j].mau != color)
			break;
		dem++;
	}
	for (tang = i + 1; tang <= 9; tang++) //Từ ô đó tiến lên
	{
		if (vt[tang][j].mau != color)
			break;
		dem++;
	}
	if (dem >= 4)
	{
		for (int k = giam + 1; k < tang; k++)
		{
			vt[k][j].mau = 0;
			delay(100);
			veHinhTron(vt[k][j].O, c / 2 - 2, 0);
		}
		tam += dem;
	}

	//Xet hang ngang
	dem = 0;
	for (giam = j - 1; giam >= 0; giam--) //Từ ô đó qua trái
	{
		if (vt[i][giam].mau != color)
			break;
		dem++;
	}
	for (tang = j + 1; tang <= 9; tang++) //Từ ô đó qua phải
	{
		if (vt[i][tang].mau != color)
			break;
		dem++;
	}
	if (dem >= 4)
	{
		for (int k = giam + 1; k < tang; k++)
		{
			vt[i][k].mau = 0;
			delay(100);
			veHinhTron(vt[i][k].O, c / 2 - 2, 0);
		}
		tam += dem;
	}

	//Xet duong cheo 1
	/*
		\||||||||
		|\|||||||
		||\||||||
		|||\|||||
		||||\||||
		|||||\|||
		||||||\||
		|||||||\|
		||||||||\
	*/
	dem = 0;
	giami = i - 1;
	giamj = j - 1;
	tangi = i + 1;
	tangj = j + 1;

	while (giami >= 0 && giamj >= 0)
	{
		if (vt[giami][giamj].mau != color)
			break;
		dem++;
		giami--;
		giamj--;
	}

	while (tangi <= 9 && tangj <= 9)
	{
		if (vt[tangi][tangj].mau != color)
			break;
		dem++;
		tangi++;
		tangj++;
	}
	if (dem >= 4)
	{
		while (giami < tangi && giamj < tangj)
		{
			vt[giami][giamj].mau = 0;
			delay(100);
			veHinhTron(vt[giami][giamj].O, c / 2 - 2, 0);
			giami++;
			giamj++;
		}
		tam += dem;
	}

	//Xet duong cheo 2
	/*
	||||||||/
	|||||||/|
	||||||/||
	|||||/|||
	||||/||||
	|||/|||||
	||/||||||
	|/|||||||
	/||||||||
	*/
	dem = 0;
	giami = i - 1;
	giamj = j - 1;
	tangi = i + 1;
	tangj = j + 1;

	while (giami >= 0 && tangj <= 9)
	{
		if (vt[giami][tangj].mau != color)
			break;
		dem++;
		giami--;
		tangj++;
	}

	while (tangi <= 9 && giamj >= 0)
	{
		if (vt[tangi][giamj].mau != color)
			break;
		dem++;
		tangi++;
		giamj--;
	}
	if (dem >= 4)
	{
		while (tangi > giami && giamj < tangj)
		{
			vt[tangi][giamj].mau = 0;
			delay(100);
			veHinhTron(vt[tangi][giamj].O, c / 2 - 4, 0);
			tangi--;
			giamj++;
		}
		tam += dem;
	}

	tam++; //o thu i, j
	switch (tam)
	{
	case 1: case 2: case 3: case 4: return false;
	case 5:
		if (mode == 1)
			score += 8;
		else if (mode == 2)
			score += 10;
		else
			score += 12;
		break;
	case 6:
		if (mode == 1)
			score += 10;
		else if (mode == 2)
			score += 12;
		else
			score += 14;
		break;
	case 7:
		if (mode == 1)
			score += 14;
		else if (mode == 2)
			score += 18;
		else
			score += 22;
		break;
	case 8:
		if (mode == 1)
			score += 18;
		else if (mode == 2)
			score += 24;
		else
			score += 28;
		break;
	case 9:
		if (mode == 1)
			score += 34;
		else if (mode == 2)
			score += 42;
		else
			score += 50;
		break;
	default:
		if (mode == 1)
			score += 48 + 14 * (tam - 10);
		else if (mode == 2)
			score += 60 + 18 * (tam - 10);
		else
			score += 72 + 22 * (tam - 10);
		break;
	}
	n -= tam;
	showScore();
	if (score > bestscore)
		saveBestScore();
	return true;
}

void canLink(int i1, int j1, int i2, int j2, int dem)
{
	diQua[i1][j1] = dem; //Số bước từ ô bắt đầu
	if (i1 == i2 && j1 == j2) //Nếu là ô đích
	{
		if (dem < minStep) //Nếu số bước lần này thấp hơn lần trước -> lưu lại
		{
			minStep = dem;
		}
		return;
	}
	int i, j;
	for (int k = 0; k < 4; k++)
	{
		i = i1 + ki[k]; //Xét 4 ô xung quanh
		j = j1 + kj[k];
		//Nếu vẫn còn trong bàn cờ, không phải ô đã có bi, chưa từng đi qua hoặc đã đi qua với số bước lớn hơn -> xét ô đó với số bước hiện tại
		if (i >= 0 && i <= 8 && j >= 0 && j <= 8 && vt[i][j].mau <= 0 && (diQua[i][j] > dem || diQua[i][j] == 0)) 
			canLink(i, j, i2, j2, dem + 1);
	}
}

void veHCN(int x, int y, int width, int height)
{
	lineDDA(x, y, x + width, y, 15);
	lineDDA(x + width, y, x + width, y + height, 15);
	lineDDA(x + width, y + height, x, y + height, 15);
	lineDDA(x, y + height, x, y, 15);
}

bool playAgain()
{
	setcolor(15);
	outtextxy(100, 425, "Do you want to play again?");
	veHCN(390, 420, 50, 25);
	veHCN(465, 420, 50, 25);
	outtextxy(400, 425, "Yes");
	outtextxy(480, 425, "No");

	int x, y;
	while (true) //luôn lặp lại đến khi break hoặc return
	{
		while (!ismouseclick(WM_LBUTTONDOWN))
		{
			delay(10);
		}
		getmouseclick(WM_LBUTTONDOWN, x, y);
		if (y >= 420 && y <= 445)
			if (x >= 390 && x <= 440) //Chọn yes
			{
				cleardevice();
				return true;
			}
		if (x >= 465 && x <= 515) //Chọn no
			return false;
	}
}

void showScore()
{
	veHCN(475, 150, 100, 25);
	setcolor(0);
	setfillstyle(1, 0);
	floodfill(478, 151, 0);
	char s[10];
	sprintf(s, "%d", score); //Chuyển số thành chuỗi
	setcolor(15);
	outtextxy(480, 155, s);
}

void showBestScore()
{
	std::ifstream ifile;
	ifile.open("bestscore.txt");
	if (ifile.is_open())
	{
		ifile >> bestscore;
		ifile.close();
	}

	veHCN(475, 125, 100, 25);
	setcolor(0);
	setfillstyle(1, 0);
	floodfill(478, 126, 0);
	char s[10];
	sprintf(s, "%d", bestscore); //Chuyển số thành chuỗi
	setcolor(15);
	outtextxy(480, 130, s);
}

void saveBestScore()
{
	std::ofstream ofile;
	ofile.open("bestscore.txt");
	if (ofile.is_open())
	{
		ofile << score;
		ofile.close();
	}
	showBestScore();
}

void showHome()
{
	cleardevice();
	setcolor(12);
	settextstyle(2, 0, 50);
	outtextxy(175, 100, "Line 1999");

	//Play
	veHCN(250, 200, 80, 50);
	setcolor(15);
	settextstyle(0, 0, 2);
	outtextxy(263, 217, "Play");

	//Exit
	veHCN(250, 300, 80, 50);
	outtextxy(263, 317, "Exit");

	int x, y;
	while (true) //Lặp lại đến khi đúng điều kiện để return
	{
		while (!ismouseclick(WM_LBUTTONDOWN)) //Chờ đến khi người dùng click chuột trái
		{
			delay(0);
		}
		getmouseclick(WM_LBUTTONDOWN, x, y); //Lấy vị trí mới nhất
		if (x > 250 && x < 330 && y > 200 && y < 250)
		{
			cleardevice();
			settextstyle(2, 0, 20);
			outtextxy(125, 25, "Choose mode");
			settextstyle(0, 0, 0);
			veHCN(250, 150, 80, 50);
			outtextxy(267, 167, "Easy");
			veHCN(250, 250, 80, 50);
			outtextxy(261, 267, "Medium");
			veHCN(250, 350, 80, 50);
			outtextxy(267, 367, "Hard");
			while (true)
			{
				while (!ismouseclick(WM_LBUTTONDOWN))
				{
					delay(0);
				}
				getmouseclick(WM_LBUTTONDOWN, x, y);
				if (x > 250 && x < 330)
					if (y > 150 && y < 200)
					{
						mode = 1;
						return;
					}
					else if (y > 250 && y < 300)
					{
						mode = 2;
						for(int i = 0; i < 3; i++)
							next[i].x = 465 + c / 2 + i * c;
						next[0].y = next[1].y = next[2].y = c + c / 2;
						return;
					}
					else if (y > 350 && y < 400)
					{
						mode = 3;
						return;
					}
			}
		}
		if (x > 250 && x < 330 && y > 300 && y < 350)
		{
			mode = -1;
			return;
		}
	}
}