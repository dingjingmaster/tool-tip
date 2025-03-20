#include "tooltip-manager.h"

#include <QMutex>
#include <QDebug>
#include <QTimer>
#include <QWidget>
#include <QScreen>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGuiApplication>

#define TIP_HIGHT_GAP           10
#define TIP_WIDGET_WIDTH        300


ToolTipManager* ToolTipManager::gInstance = nullptr;

ToolTipManager *ToolTipManager::getInstance()
{
    if (!gInstance) {
        static QMutex lock;
        lock.lock();
        if (!gInstance) {
            gInstance = new ToolTipManager();
        }
        lock.unlock();
    }

    return gInstance;
}

void ToolTipManager::showMessage(QString msg)
{
    if (msg.endsWith("\r\n")) {
        msg.chop(2);
    }
    else if (msg.endsWith("\n")) {
        msg.chop(1);
    }

    if (msg.isEmpty() || msg == "") {
        return;
    }

    auto w = std::make_shared<TipWrap>(msg);
    w.get()->connect (w.get(), &TipWrap::closeTip, this, [=] () {
        for (auto& it : mMainWidgets) {
            if (it.get() == sender()) {
                closeTip(it);
                break;
            }
        }
    });
    mMainLayout->addWidget(w.get());
    mMainWidgets.append(w);
    checkTipWidgets();
    if (!mMainTimer->isActive()) {
        mMainTimer->start();
    }
    w->show();
    if (!isVisible()) {
        show();
    }
}

ToolTipManager::~ToolTipManager()
{
}

ToolTipManager::ToolTipManager(QWidget* parent)
    : QWidget{parent}, mMainLayout(new QVBoxLayout), mMainTimer(new QTimer)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    // setAttribute(Qt::WA_X11NetWmWindowTypeNotification);
    setStyleSheet("background-color: transparent;");
    mMainLayout->setSpacing(TIP_HIGHT_GAP);
    mMainLayout->setContentsMargins(0, 0, 0, 0);
    qobject_cast<QVBoxLayout*>(mMainLayout)->addStretch();
    setLayout(mMainLayout);

    auto rect = qApp->primaryScreen()->availableGeometry();
    auto tl = rect.topLeft();
    auto br = rect.bottomRight();

    mMainPos.setX(br.x() - TIP_WIDGET_WIDTH - 3);
    mMainPos.setY(tl.y());
    mMainPos.setWidth(TIP_WIDGET_WIDTH);
    mMainPos.setHeight(br.y() - tl.y() - 10);

    mMaxTip = (mMainPos.height() - 10) / (TIP_WIDGET_WIDTH * 0.6 + TIP_HIGHT_GAP);

    connect (mMainTimer, &QTimer::timeout, this, [=] () {
        checkTipWidgets();
        for (auto& w : mMainWidgets) {
            ++w->mCurrentSec;
            if (w->mCurrentSec >= w->mTotalSec) {
                closeTip(w);
            }
        }

        if (mMainWidgets.isEmpty()) {
            mMainTimer->stop();
            hide();
        }
    });
    mMainTimer->setInterval(1000);
}

void ToolTipManager::closeTip(std::shared_ptr<TipWrap> tip)
{
    tip->hide();
    mMainLayout->removeWidget(tip.get());
    mMainWidgets.removeOne(tip);
}

void ToolTipManager::checkTipWidgets()
{
    auto all = mMainWidgets.count() - mMaxTip;
    for (int idx = 0; idx < all; ++idx) {
        auto it = mMainWidgets.at(idx);
        closeTip(it);
    }
}

void ToolTipManager::showEvent(QShowEvent *event)
{
    setGeometry(mMainPos);
}

TipWidget::TipWidget(const QString& msg, QWidget* parent)
    : QLabel(parent)
{
    setText(msg);
    setWordWrap(true);
    setOpenExternalLinks(true);
    setTextFormat(Qt::RichText);
}

TipWrap::TipWrap(const QString &msg, QWidget *parent)
    : QWidget(parent), mWidget(new TipWidget(msg, parent)), mTotalSec(300)
{
    setContentsMargins(6, 6, 6, 6);
    setFixedSize(TIP_WIDGET_WIDTH, TIP_WIDGET_WIDTH * 0.6);
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background-color: rgba(255,255,255,1); color: rgba(0,0,0,1);");

    auto layout = new QVBoxLayout;
    auto l2 = new QHBoxLayout;
    auto cancel = new QPushButton;
    cancel->setText(tr("Close"));
    cancel->setContentsMargins(9, 9, 9, 9);
    cancel->setAttribute(Qt::WA_StyledBackground);
    cancel->setStyleSheet("QPushButton {"
                          " background-color: rgba(255,255,255,1);"
                          " color: rgba(0,0,0,1);"
                          " border: 1px solid #a8a8a8;"
                          "}");

    connect (cancel, &QPushButton::clicked, this, [=] (bool) { Q_EMIT closeTip(); });

    layout->addWidget(mWidget);
    l2->addStretch();
    l2->addWidget(cancel);
    l2->addStretch();
    layout->addLayout(l2);
    setLayout(layout);
}
