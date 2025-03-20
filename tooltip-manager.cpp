#include "tooltip-manager.h"

#include <QMutex>
#include <QDebug>
#include <QTimer>
#include <QWidget>
#include <QScreen>
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

    auto w = std::make_shared<TipWidget>(msg);
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
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_X11NetWmWindowTypeNotification);
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

    mMaxTip = (mMainPos.height() - 10) / (TIP_WIDGET_WIDTH * 0.6 + TIP_HIGHT_GAP) - 1;
    qInfo() << mMaxTip;

    connect (mMainTimer, &QTimer::timeout, this, [=] () {
        checkTipWidgets();
        for (auto& w : mMainWidgets) {
            ++w->mCurrentSec;
            if (w->mCurrentSec >= w->mTotalSec) {
                w->hide();
                mMainLayout->removeWidget(w.get());
                mMainWidgets.removeOne(w);
            }
        }

        if (mMainWidgets.isEmpty()) {
            mMainTimer->stop();
            hide();
        }
    });
    mMainTimer->setInterval(1000);
}

void ToolTipManager::checkTipWidgets()
{
    auto all = mMainWidgets.count() - mMaxTip;
    for (int idx = 0; idx < all; ++idx) {
        mMainLayout->removeWidget(mMainWidgets.at(idx).get());
        mMainWidgets.at(idx)->hide();
        mMainWidgets.removeAt(idx);
    }
}

void ToolTipManager::showEvent(QShowEvent *event)
{
    setGeometry(mMainPos);
}

TipWidget::TipWidget(const QString& msg, QWidget* parent)
    : QLabel(parent), mTotalSec(3)
{
    setContentsMargins(6, 6, 6, 6);
    setFixedSize(TIP_WIDGET_WIDTH, TIP_WIDGET_WIDTH * 0.6);
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background-color: rgba(255,255,255,1);");

    setText(msg);
    setWordWrap(true);
    setOpenExternalLinks(true);
    setTextFormat(Qt::RichText);
}
