#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QString>
#include <QFont>
#include <QFrame>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // --- State ---
    int scoreA = 0;
    int scoreB = 0;

    // --- Main window ---
    QWidget window;
    window.setWindowTitle("Scoreboard");
    window.setMinimumSize(420, 280);

    // --- Fonts ---
    QFont titleFont("Arial", 14, QFont::Bold);
    QFont scoreFont("Arial", 36, QFont::Bold);
    QFont btnFont("Arial", 16, QFont::Bold);

    // --- Player A widgets ---
    QLabel *labelA = new QLabel("Player A", &window);
    labelA->setFont(titleFont);
    labelA->setAlignment(Qt::AlignCenter);
    labelA->setStyleSheet("color: #c0392b;");

    QLabel *scoreALabel = new QLabel("0", &window);
    scoreALabel->setFont(scoreFont);
    scoreALabel->setAlignment(Qt::AlignCenter);
    scoreALabel->setMinimumWidth(120);
    scoreALabel->setStyleSheet(
        "background-color: #f0f0f0;"
        "border: 2px solid #c0392b;"
        "border-radius: 8px;"
        "padding: 8px;"
    );

    QPushButton *minusA = new QPushButton("−", &window);
    QPushButton *plusA  = new QPushButton("+", &window);
    minusA->setFont(btnFont);
    plusA->setFont(btnFont);
    minusA->setFixedSize(55, 40);
    plusA->setFixedSize(55, 40);
    minusA->setStyleSheet("background-color: #e74c3c; color: white; border-radius: 6px;");
    plusA->setStyleSheet("background-color: #27ae60; color: white; border-radius: 6px;");

    // --- Player B widgets ---
    QLabel *labelB = new QLabel("Player B", &window);
    labelB->setFont(titleFont);
    labelB->setAlignment(Qt::AlignCenter);
    labelB->setStyleSheet("color: #2980b9;");

    QLabel *scoreBLabel = new QLabel("0", &window);
    scoreBLabel->setFont(scoreFont);
    scoreBLabel->setAlignment(Qt::AlignCenter);
    scoreBLabel->setMinimumWidth(120);
    scoreBLabel->setStyleSheet(
        "background-color: #f0f0f0;"
        "border: 2px solid #2980b9;"
        "border-radius: 8px;"
        "padding: 8px;"
    );

    QPushButton *minusB = new QPushButton("−", &window);
    QPushButton *plusB  = new QPushButton("+", &window);
    minusB->setFont(btnFont);
    plusB->setFont(btnFont);
    minusB->setFixedSize(55, 40);
    plusB->setFixedSize(55, 40);
    minusB->setStyleSheet("background-color: #e74c3c; color: white; border-radius: 6px;");
    plusB->setStyleSheet("background-color: #27ae60; color: white; border-radius: 6px;");

    // --- Reset button ---
    QPushButton *resetBtn = new QPushButton("Reset", &window);
    resetBtn->setFont(QFont("Arial", 12, QFont::Bold));
    resetBtn->setFixedHeight(38);
    resetBtn->setStyleSheet(
        "background-color: #7f8c8d; color: white; border-radius: 6px; padding: 0 20px;"
    );

    // --- Divider ---
    QFrame *divider = new QFrame(&window);
    divider->setFrameShape(QFrame::VLine);
    divider->setFrameShadow(QFrame::Sunken);

    // --- updateUi lambda ---
    auto updateUi = [&]() {
        scoreALabel->setText(QString::number(scoreA));
        scoreBLabel->setText(QString::number(scoreB));
    };

    // --- Connections ---
    QObject::connect(plusA,  &QPushButton::clicked, [&](){ scoreA++; updateUi(); });
    QObject::connect(minusA, &QPushButton::clicked, [&](){ scoreA--; updateUi(); });
    QObject::connect(plusB,  &QPushButton::clicked, [&](){ scoreB++; updateUi(); });
    QObject::connect(minusB, &QPushButton::clicked, [&](){ scoreB--; updateUi(); });
    QObject::connect(resetBtn, &QPushButton::clicked, [&](){
        scoreA = 0;
        scoreB = 0;
        updateUi();
    });

    // --- Layout: Player A column ---
    QVBoxLayout *colA = new QVBoxLayout;
    colA->setSpacing(8);
    colA->addWidget(labelA);
    colA->addWidget(scoreALabel);
    QHBoxLayout *btnRowA = new QHBoxLayout;
    btnRowA->setSpacing(8);
    btnRowA->addStretch();
    btnRowA->addWidget(minusA);
    btnRowA->addWidget(plusA);
    btnRowA->addStretch();
    colA->addLayout(btnRowA);

    // --- Layout: Player B column ---
    QVBoxLayout *colB = new QVBoxLayout;
    colB->setSpacing(8);
    colB->addWidget(labelB);
    colB->addWidget(scoreBLabel);
    QHBoxLayout *btnRowB = new QHBoxLayout;
    btnRowB->setSpacing(8);
    btnRowB->addStretch();
    btnRowB->addWidget(minusB);
    btnRowB->addWidget(plusB);
    btnRowB->addStretch();
    colB->addLayout(btnRowB);

    // --- Main layout ---
    QHBoxLayout *playersRow = new QHBoxLayout;
    playersRow->setSpacing(16);
    playersRow->addLayout(colA);
    playersRow->addWidget(divider);
    playersRow->addLayout(colB);

    QHBoxLayout *resetRow = new QHBoxLayout;
    resetRow->addStretch();
    resetRow->addWidget(resetBtn);
    resetRow->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(&window);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);
    mainLayout->addLayout(playersRow);
    mainLayout->addLayout(resetRow);

    window.show();
    return app.exec();
}
