#include <fstream>
#include <iostream>
using namespace std;

class enhancement {
public:
    int numR;
    int numC;
    int minV;
    int maxV;
    int maskR;
    int maskC;
    int maskMin;
    int maskMax;
    int** mirrorFramedAry;
    int** avgAry;
    int** GaussAry;
    int neighborAry[25];
    int maskAry[25];
    int maskWeight;
    int* hisAvgAry;
    int* histGaussAry;

    enhancement() {

    }
    void loadImage(ifstream& in) {
        for (int i = 2; i < numR; i++) {
            for (int j = 2; j < numC; j++) {
                in >> mirrorFramedAry[i][j];
            }
        }
    }

    void mirrorFraming() {
        for (int i = 0; i < numC + 4; i++) {
            mirrorFramedAry[0][i] = mirrorFramedAry[2][i];
            mirrorFramedAry[1][i] = mirrorFramedAry[2][i];
            mirrorFramedAry[numR][i] = mirrorFramedAry[numR - 1][i];
            mirrorFramedAry[numR + 1][i] = mirrorFramedAry[numR - 1][i];
        }

        for (int i = 0; i < numR + 4; i++) {
            mirrorFramedAry[i][0] = mirrorFramedAry[i][2];
            mirrorFramedAry[i][1] = mirrorFramedAry[i][2];
            mirrorFramedAry[i][numC] = mirrorFramedAry[i][numC - 1];
            mirrorFramedAry[i][numC + 1] = mirrorFramedAry[i][numC - 1];
        }
    }

    int loadMaskAry(ifstream& m) {
        int mask;
        int sum = 0;
        m >> maskR;
        m >> maskC;
        m >> maskMin;
        m >> maskMax;
        for (int i = 0; i < 25; i++) {
            m >> mask;
            sum += mask;
            maskAry[i] = mask;
        }
        return sum;
    }

    void loadNeighborAry(int i, int j) {
        int z = 0;
        for (int x = i - 2; x < i + 3; x++) {
            for (int y = j - 2; y < j + 3; y++) {
                neighborAry[z] = mirrorFramedAry[x][y];
                z++;
            }
        }
    }

    int avg5x5(int i, int j) {
        loadNeighborAry(i, j);
        int sum = 0;
        for (int i = 0; i < 25; i++) {
            sum += neighborAry[i];
        }
        int avg = sum / 25;
        return sum;
    }

    void computeGauss5x5(ofstream& debug) {
        debug << "entering computeGauss method" << endl;
        int i = 2;
        while (i < numR + 2) {
            int j = 2;
            while (j < numC + 2) {
                loadNeighborAry(i, j);
                GaussAry[i][j] = convolution(debug);
                j++;
            }
            i++;
        }
        debug << "Leaving computeGauss method" << endl;
    }

    void printHist(int* h, ofstream& hf, ofstream& debug) {
        debug << "entering printHist method" << endl;
        hf << numR << " " << numC << " " << minV << " " << maxV << endl;
        int i = 0;
        while (i <= maxV) {
            hf << i << " " << h[i] << endl;
            i++;
        }
        debug << "Leaving printHist method" << endl;
    }

    int convolution(ofstream& debug) {
        debug << "entering convolution method" << endl;
        int r = 0;
        int i = 0;
        while (i < 25) {
            r += neighborAry[i] * maskAry[i];
            i++;
        }
        debug << "in convolution method, result is: " << r << endl;
        debug << "Leaving convolution method" << endl;
        return r / maskWeight;
    }

    void computeAvg5x5(ofstream& debug) {
        debug << "entering computeAvg5x5 method" << endl;
        int i = 2;
        while (i < numR + 2) {
            int j = 2;
            while (j < numC + 2) {
                avgAry[i][j] = avg5x5(i, j);
                j++;
            }
            i++;
        }
        debug << "Leaving computeAvg5x5 method" << endl;
    }

    void imgReformat(int** inAry, ofstream& out) {
        out << numR << " " << numC << " " << minV << " " << maxV << endl;
        string s = "" + (maxV);

        int w = s.length();
        int r = 2;
        while (r < numR + 2) {
            int c = 2;
            while (c < numC + 2) {
                out << inAry[r][c];
                s = "" + (inAry[r][c]);
                int ww = s.length();
                while (ww < w) {
                    out << " ";
                    ww++;
                }
                c++;
            }
            r++;
        }
    }

    void computeHist(int** in, int* histAry, ofstream& debug) {
        debug << "entering computeHist method" << endl;
        int i = 2;
        while (i < numR + 2) {
            int j = 2;
            while (j < numC + 2) {
                histAry[avgAry[i][j]]++;
                j++;
            }
            i++;
        }

        debug << "Leaving computeHist method" << endl;
    }
};

int main(int argc, char* argv[]) {
    ifstream in(argv[1]);
    ifstream mask(argv[2]);
    ifstream choice(argv[3]);
    ofstream out(argv[4]);
    ofstream debug(argv[5]);
    enhancement e = enhancement();
    int ch;
    choice >> ch;
    in >> e.numR >> e.numC >> e.minV >> e.maxV;
    mask >> e.maskR >> e.maskC >> e.maskMin >> e.maskMax;
    e.mirrorFramedAry = new int* [(e.numR + 4) * (e.numC + 4)];
    e.avgAry = new int* [(e.numR + 4) * (e.numC + 4)];
    e.GaussAry = new int* [(e.numR + 4) * (e.numC + 4)];
    e.hisAvgAry = new int[e.maxV + 1];
    e.histGaussAry = new int[e.maxV + 1];
    e.loadImage(in);
    e.mirrorFraming();
    e.imgReformat(e.mirrorFramedAry, out);
    if (ch == 1) {
        e.computeAvg5x5(debug);
        e.computeHist(e.avgAry, e.hisAvgAry, debug);
        string nameAvg = (string)argv[1] + "_Avg5x5.txt";
        ofstream avgFile(nameAvg);
        e.imgReformat(e.avgAry, out);
        avgFile << e.numR << " " << e.numC << " " << e.minV << " " << e.maxV << endl;
        for (int i = 2; i < e.numR + 2; i++) {
            for (int j = 2; j < e.numC + 2; j++) {
                avgFile << e.avgAry[i][j] << " ";
            }
            avgFile << endl;
        }
        string avgHist = (string)argv[1] + "_Avg5x5_hist.txt";
        ofstream histAvgFile(avgHist);
        e.printHist(e.hisAvgAry, histAvgFile, debug);
        histAvgFile.close();
    }
    if (ch == 2) {
        e.maskWeight = e.loadMaskAry(mask);
        e.computeGauss5x5(debug);
        e.computeHist(e.GaussAry, e.histGaussAry, debug);
        string nameGauss = (string)argv[1] + "_Gauss5x5.txt";
        ofstream GaussFile(nameGauss);
        e.imgReformat(e.GaussAry, out);
        GaussFile << e.numR << " " << e.numC << " " << e.minV << " " << e.maxV << endl;
        for (int i = 2; i < e.numR + 2; i++) {
            for (int j = 2; j < e.numC + 2; j++) {
                GaussFile << e.avgAry[i][j] << " ";
            }
            GaussFile << endl;
        }
        string GaussHist = (string)argv[1] + "_Gauss5x5_hist.txt";
        ofstream histGaussFile(GaussHist);
        e.printHist(e.histGaussAry, histGaussFile, debug);
        histGaussFile.close();
    }
    out.close();
    debug.close();
}