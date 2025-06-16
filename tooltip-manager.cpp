#include "tooltip-manager.h"

#include <QMutex>
#include <QDebug>
#include <QTimer>
#include <QWidget>
#include <QScreen>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGuiApplication>
#include <QPainter>
#include <QMouseEvent>

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
    setStyleSheet("background-color: transparent;");
    mMainLayout->setSpacing(TIP_HIGHT_GAP);
    mMainLayout->setContentsMargins(0, 0, 0, 0);
    qobject_cast<QVBoxLayout*>(mMainLayout)->addStretch();
    setLayout(mMainLayout);

    auto rect = qApp->primaryScreen()->availableGeometry();
    auto tl = rect.topLeft();
    auto br = rect.bottomRight();

    setFixedWidth(TIP_WIDGET_WIDTH);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

    mMainPos.setX(br.x() - TIP_WIDGET_WIDTH - 3);
    mMainPos.setY(tl.y());
    mMainPos.setWidth(TIP_WIDGET_WIDTH);

    mMaxTip = (br.y() - tl.y() - 20) / (TIP_WIDGET_WIDTH * 0.6 + TIP_HIGHT_GAP);

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
    resize(0, 0);
}

void ToolTipManager::checkTipWidgets()
{
    auto all = mMainWidgets.count() - mMaxTip;
    for (int idx = 0; idx < all; ++idx) {
        auto it = mMainWidgets.at(idx);
        closeTip(it);
    }
}

void ToolTipManager::resizeEvent(QResizeEvent *event)
{
    auto rect = qApp->primaryScreen()->availableGeometry();
    auto br = rect.bottomRight();

    int w = (mMainLayout->count() - 1) * (TIP_WIDGET_WIDTH * 0.6 + TIP_HIGHT_GAP);

    qInfo() << "c: " << mMainLayout->count() - 1 << " h: " << height() << " w: " << width();

    mMainPos.setHeight(w);
    mMainPos.setY(br.y() - w - 10);
    setGeometry(mMainPos);
}

TipWidget::TipWidget(const QString& msg, QWidget* parent)
    : QLabel(parent)
{
    setContentsMargins(8, 8, 8, 3);
    setText(msg);
    setWordWrap(true);
    setOpenExternalLinks(true);
    setTextFormat(Qt::RichText);
}

TipWrap::TipWrap(const QString &msg, QWidget *parent)
    : QWidget(parent), mWidget(new TipWidget(msg, parent)), mTotalSec(300)
{
    setContentsMargins(0, 0, 0, 12);
    setFixedSize(TIP_WIDGET_WIDTH, TIP_WIDGET_WIDTH * 0.6);
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background-color: rgba(255,255,255,1); color: rgba(0,0,0,1);");

    mHeader = new TipHeader;

    auto layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(mHeader);
    auto l2 = new QHBoxLayout;
    auto cancel = new QPushButton;
    QString str(tr("Close"));
    QFont f = font();
    f.setBold(true);
    cancel->setFont(f);
    cancel->setText(str);
    cancel->setCursor(Qt::PointingHandCursor);
    cancel->setAttribute(Qt::WA_StyledBackground);
    cancel->setStyleSheet("QPushButton {"
                          " border: none;"
                          " outline: none;"
                          " border-radius: 5px;"
                          " padding: 1px 6px 1px 6px;"
                          " border: 1px solid #a8a8a8;"
                          " color: rgba(255,255,255,255);"
                          " background-color: rgba(28,167,217,255);"
                          "}"
                          "");
    cancel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    cancel->adjustSize();

    connect (cancel, &QPushButton::clicked, this, [=] (bool) { Q_EMIT closeTip(); });
    connect (mHeader, &TipHeader::close, this, &TipWrap::closeTip);

    layout->addWidget(mWidget);
    l2->addStretch();
    l2->addWidget(cancel);
    l2->addStretch();
    layout->addLayout(l2);
    setLayout(layout);
}

TipHeader::TipHeader(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setContentsMargins(0, 0, 0, 0);
    setFixedHeight(42);
    mTitle = QString(tr("Process approval"));
}

void TipHeader::paintEvent(QPaintEvent *e)
{
    int h = height();
    int w = width();

    QPainter p(this);
    p.save();
    p.fillRect(rect(), QColor(28, 167, 217));
    p.restore();

    p.save();
    QPen pen = p.pen();
    pen.setColor(Qt::white);
    p.setPen(pen);
    QFont f = font();
    f.setBold(true);
    QFontMetrics fm(f);
    int fh = fm.height();
    QRect title = rect();
    title.setX(8);
    title.setY((h - fh) / 2);
    title.setHeight(fh);
    title.setWidth(fm.horizontalAdvance(mTitle));
    p.drawText(title, Qt::AlignCenter, mTitle);
    p.restore();

    p.save();
    QRect cls = rect();
    cls.setX(w - 24);
    cls.setY((h - 16) / 2);
    cls.setHeight(16);
    cls.setWidth(16);
    p.drawImage(cls, QImage(":/data/icons/close.png"));
    p.restore();

    qDebug() << "w: " << width() << ", h: " << height() << ", fm.h: " << fh;
}

void TipHeader::mouseMoveEvent(QMouseEvent *e)
{
    int h = height();
    int w = width();

    QRect cls = rect();
    cls.setX(w - 24);
    cls.setY((h - 16) / 2);
    cls.setHeight(16);
    cls.setWidth(16);
    if (cls.contains(e->pos())) {
        setCursor(Qt::PointingHandCursor);
    }
    else {
        setCursor(Qt::ArrowCursor);
    }
}

void TipHeader::mousePressEvent(QMouseEvent *e)
{
    int h = height();
    int w = width();

    QRect cls = rect();
    cls.setX(w - 24);
    cls.setY((h - 16) / 2);
    cls.setHeight(16);
    cls.setWidth(16);
    if (cls.contains(e->pos())) {
        Q_EMIT close();
    }
}
