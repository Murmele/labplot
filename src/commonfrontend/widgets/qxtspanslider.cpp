/****************************************************************************
**
** Copyright (C) Qxt Foundation. Some rights reserved.
**
** This file is part of the QxtGui module of the Qxt library.
**
** This library is free software; you can redistribute it and/or modify it
** under the terms of the Common Public License, version 1.0, as published by
** IBM.
**
** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
** FITNESS FOR A PARTICULAR PURPOSE.
**
** You should have received a copy of the CPL along with this file.
** See the LICENSE file and the cpl1.0.txt file included with the source
** distribution for more information. If you did not receive a copy of the
** license, contact the Qxt Foundation.
**
** <http://libqxt.org>  <foundation@libqxt.org>
**
****************************************************************************/
#include "qxtspanslider.h"
#include "qxtspanslider_p.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionSlider>

QxtSpanSliderPrivate::QxtSpanSliderPrivate()
        : lower(0),
        upper(0),
        offset(0),
        position(0),
        lastPressed(NoHandle),
        mainControl(LowerHandle),
        lowerPressed(QStyle::SC_None),
        upperPressed(QStyle::SC_None)
{}

// TODO: get rid of this in Qt 4.3
void QxtSpanSliderPrivate::initStyleOption(QStyleOptionSlider* option, SpanHandle handle) const {
    if (!option)
        return;

    const QSlider* p = &qxt_p();
    option->initFrom(p);
    option->subControls = QStyle::SC_None;
    option->activeSubControls = QStyle::SC_None;
    option->orientation = p->orientation();
    option->maximum = p->maximum();
    option->minimum = p->minimum();
    option->tickPosition = p->tickPosition();
    option->tickInterval = p->tickInterval();
    option->upsideDown = (p->orientation() == Qt::Horizontal) ?
                         (p->invertedAppearance() != (option->direction == Qt::RightToLeft)) : (!p->invertedAppearance());
    option->direction = Qt::LeftToRight; // we use the upsideDown option instead
    option->sliderPosition = (handle == LowerHandle ? lower : upper);
    option->sliderValue = (handle == LowerHandle ? lower : upper);
    option->singleStep = p->singleStep();
    option->pageStep = p->pageStep();
    if (p->orientation() == Qt::Horizontal)
        option->state |= QStyle::State_Horizontal;
}

int QxtSpanSliderPrivate::pixelPosToRangeValue(int pos) const {
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    int sliderMin = 0;
    int sliderMax = 0;
    int sliderLength = 0;
    const QSlider* p = &qxt_p();
    const QRect gr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
    const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
    if (p->orientation() == Qt::Horizontal)
    {
        sliderLength = sr.width();
        sliderMin = gr.x();
        sliderMax = gr.right() - sliderLength + 1;
    }
    else
    {
        sliderLength = sr.height();
        sliderMin = gr.y();
        sliderMax = gr.bottom() - sliderLength + 1;
    }
    return QStyle::sliderValueFromPosition(p->minimum(), p->maximum(), pos - sliderMin,
                                           sliderMax - sliderMin, opt.upsideDown);
}

void QxtSpanSliderPrivate::handleMousePress(const QPoint& pos, QStyle::SubControl& control, int value, SpanHandle handle)
{
    QStyleOptionSlider opt;
    initStyleOption(&opt, handle);
    QSlider* p = &qxt_p();
    const QStyle::SubControl oldControl = control;
    control = p->style()->hitTestComplexControl(QStyle::CC_Slider, &opt, pos, p);
    const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
    if (control == QStyle::SC_SliderHandle)
    {
        position = value;
        offset = pick(pos - sr.topLeft());
        lastPressed = handle;
        p->setSliderDown(true);
    }
    if (control != oldControl)
        p->update(sr);
}

void QxtSpanSliderPrivate::setupPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const
{
    QColor highlight = qxt_p().palette().color(QPalette::Highlight);
    QLinearGradient gradient(x1, y1, x2, y2);
    gradient.setColorAt(0, highlight.dark(120));
    gradient.setColorAt(1, highlight.light(108));
    painter->setBrush(gradient);

    if (orientation == Qt::Horizontal)
        painter->setPen(QPen(highlight.dark(130), 0));
    else
        painter->setPen(QPen(highlight.dark(150), 0));
}

void QxtSpanSliderPrivate::drawSpan(QStylePainter* painter, const QRect& rect) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    const QSlider* p = &qxt_p();

    // area
    QRect groove = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
    if (opt.orientation == Qt::Horizontal)
        groove.adjust(0, 0, -1, 0);
    else
        groove.adjust(0, 0, 0, -1);

    // pen & brush
    painter->setPen(QPen(p->palette().color(QPalette::Dark).light(110), 0));
    if (opt.orientation == Qt::Horizontal)
        setupPainter(painter, opt.orientation, groove.center().x(), groove.top(), groove.center().x(), groove.bottom());
    else
        setupPainter(painter, opt.orientation, groove.left(), groove.center().y(), groove.right(), groove.center().y());

    // draw groove
#if QT_VERSION >= 0x040200
    painter->drawRect(rect.intersected(groove));
#else // QT_VERSION < 0x040200
    painter->drawRect(rect.intersect(groove));
#endif // QT_VERSION
}

void QxtSpanSliderPrivate::drawHandle(QStylePainter* painter, SpanHandle handle) const
{
    QStyleOptionSlider opt;
    initStyleOption(&opt, handle);
    opt.subControls = QStyle::SC_SliderHandle;
    QStyle::SubControl pressed = (handle == LowerHandle ? lowerPressed : upperPressed);
    if (pressed == QStyle::SC_SliderHandle)
    {
        opt.activeSubControls = pressed;
        opt.state |= QStyle::State_Sunken;
    }
    painter->drawComplexControl(QStyle::CC_Slider, opt);
}

void QxtSpanSliderPrivate::triggerAction(QAbstractSlider::SliderAction action, bool main) {
    int value = 0;
    bool up = false;
    const int min = qxt_p().minimum();
    const int max = qxt_p().maximum();
    const SpanHandle altControl = (mainControl == LowerHandle ? UpperHandle : LowerHandle);
    switch (action) {
    case QAbstractSlider::SliderSingleStepAdd:
        if ((main && mainControl == UpperHandle) || (!main && altControl == UpperHandle)) {
            value = qBound(min, upper + qxt_p().singleStep(), max);
            up = true;
            break;
        }
        value = qBound(min, lower + qxt_p().singleStep(), max);
        break;
    case QAbstractSlider::SliderSingleStepSub:
        if ((main && mainControl == UpperHandle) || (!main && altControl == UpperHandle)) {
            value = qBound(min, upper - qxt_p().singleStep(), max);
            up = true;
            break;
        }
        value = qBound(min, lower - qxt_p().singleStep(), max);
        break;
    case QAbstractSlider::SliderToMinimum:
        value = min;
        if ((main && mainControl == UpperHandle) || (!main && altControl == UpperHandle))
            up = true;
        break;
    case QAbstractSlider::SliderToMaximum:
        value = max;
        if ((main && mainControl == UpperHandle) || (!main && altControl == UpperHandle))
            up = true;
        break;
    case QAbstractSlider::SliderPageStepAdd:
    case QAbstractSlider::SliderNoAction:
    case QAbstractSlider::SliderPageStepSub:
    case QAbstractSlider::SliderMove:
        qWarning("QxtSpanSliderPrivate::triggerAction: Unknown action");
    }

    if (!up) {
        if (value > upper) {
            swapControls();
            qxt_p().setUpperValue(value);
        } else
            qxt_p().setLowerValue(value);
    } else {
        if (value < lower) {
            swapControls();
            qxt_p().setLowerValue(value);
        } else
            qxt_p().setUpperValue(value);
    }
}

void QxtSpanSliderPrivate::swapControls() {
    qSwap(lower, upper);
    qSwap(lowerPressed, upperPressed);
    lastPressed = (lastPressed == LowerHandle ? UpperHandle : LowerHandle);
    mainControl = (mainControl == LowerHandle ? UpperHandle : LowerHandle);
}

void QxtSpanSliderPrivate::updateRange(int min, int max) {
    Q_UNUSED(min);
    Q_UNUSED(max);
    // setSpan() takes care of keeping span in range
    qxt_p().setSpan(lower, upper);
}

/*!
    \class QxtSpanSlider QxtSpanSlider
    \ingroup QxtGui
    \brief A QSlider with two handles.

    QxtSpanSlider is a slider with two handles. QxtSpanSlider is
    handy for letting user to choose an span between min/max.

    The span color is calculated based on \b QPalette::Highlight.

    The keys are bound according to the following table:
    <table>
    <tr><td><b>Orientation</b></td><td><b>Key</b></td><td><b>Handle</b></td></tr>
    <tr><td>Qt::Horizontal</td><td>Qt::Key_Left</td><td>lower</td></tr>
    <tr><td>Qt::Horizontal</td><td>Qt::Key_Right</td><td>lower</td></tr>
    <tr><td>Qt::Horizontal</td><td>Qt::Key_Up</td><td>upper</td></tr>
    <tr><td>Qt::Horizontal</td><td>Qt::Key_Down</td><td>upper</td></tr>
    <tr><td>Qt::Vertical</td><td>Qt::Key_Up</td><td>lower</td></tr>
    <tr><td>Qt::Vertical</td><td>Qt::Key_Down</td><td>lower</td></tr>
    <tr><td>Qt::Vertical</td><td>Qt::Key_Left</td><td>upper</td></tr>
    <tr><td>Qt::Vertical</td><td>Qt::Key_Right</td><td>upper</td></tr>
    </table>

    Keys are bound by the time the slider is created. A key is bound
    to same handle for the lifetime of the slider. So even if the handle
    representation might change from lower to upper, the same key binding
    remains.

    \image html qxtspanslider.png "QxtSpanSlider in Plastique style."

    \note QxtSpanSlider inherits \b QSlider for implementation specific
    reasons. Adjusting any single handle specific properties like
    <ul>
    <li>\b QAbstractSlider::sliderPosition</li>
    <li>\b QAbstractSlider::value</li>
    </ul>
    has no effect. However, all slider specific properties like
    <ul>
    <li>\b QAbstractSlider::invertedAppearance</li>
    <li>\b QAbstractSlider::invertedControls</li>
    <li>\b QAbstractSlider::minimum</li>
    <li>\b QAbstractSlider::maximum</li>
    <li>\b QAbstractSlider::orientation</li>
    <li>\b QAbstractSlider::pageStep</li>
    <li>\b QAbstractSlider::singleStep</li>
    <li>\b QSlider::tickInterval</li>
    <li>\b QSlider::tickPosition</li>
    </ul>
    are taken into consideration.
 */

/*!
    \fn QxtSpanSlider::lowerValueChanged(int lower)

    This signal is emitted whenever the lower value has changed.
 */

/*!
    \fn QxtSpanSlider::upperValueChanged(int upper)

    This signal is emitted whenever the upper value has changed.
 */

/*!
    \fn QxtSpanSlider::spanChanged(int lower, int upper)

    This signal is emitted whenever the span has changed.
 */

/*!
    Constructs a new QxtSpanSlider with \a parent.
 */
QxtSpanSlider::QxtSpanSlider(QWidget* parent) : QSlider(parent)
{
    QXT_INIT_PRIVATE(QxtSpanSlider);
    connect(this, SIGNAL(rangeChanged(int,int)), &qxt_d(), SLOT(updateRange(int,int)));
}

/*!
    Constructs a new QxtSpanSlider with \a orientation and \a parent.
 */
QxtSpanSlider::QxtSpanSlider(Qt::Orientation orientation, QWidget* parent) : QSlider(orientation, parent)
{
    QXT_INIT_PRIVATE(QxtSpanSlider);
    connect(this, SIGNAL(rangeChanged(int,int)), &qxt_d(), SLOT(updateRange(int,int)));
}

/*!
    Destructs the slider.
 */
QxtSpanSlider::~QxtSpanSlider()
{}

/*!
    \property QxtSpanSlider::lowerValue
    \brief This property holds the lower value of the span
 */
int QxtSpanSlider::lowerValue() const
{
    return qMin(qxt_d().lower, qxt_d().upper);
}

void QxtSpanSlider::setLowerValue(int lower)
{
    setSpan(lower, qxt_d().upper);
}

/*!
    \property QxtSpanSlider::upperValue
    \brief This property holds the upper value of the span
 */
int QxtSpanSlider::upperValue() const
{
    return qMax(qxt_d().lower, qxt_d().upper);
}

void QxtSpanSlider::setUpperValue(int upper)
{
    setSpan(qxt_d().lower, upper);
}

/*!
    Sets the span from \a lower to \a upper.
    \sa upperValue, lowerValue
 */
void QxtSpanSlider::setSpan(int lower, int upper)
{
    const int low = qBound(minimum(), qMin(lower, upper), maximum());
    const int upp = qBound(minimum(), qMax(lower, upper), maximum());
    if (low != qxt_d().lower || upp != qxt_d().upper)
    {
        if (low != qxt_d().lower)
        {
            qxt_d().lower = low;
            emit lowerValueChanged(low);
        }
        if (upp != qxt_d().upper)
        {
            qxt_d().upper = upp;
            emit upperValueChanged(upp);
        }
        emit spanChanged(qxt_d().lower, qxt_d().upper);
        update();
    }
}

/*!
    \reimp
 */
void QxtSpanSlider::keyPressEvent(QKeyEvent* event)
{
    QSlider::keyPressEvent(event);

    bool main = true;
    SliderAction action = SliderNoAction;
    switch (event->key())
    {
    case Qt::Key_Left:
        main   = (orientation() == Qt::Horizontal);
        action = !invertedAppearance() ? SliderSingleStepSub : SliderSingleStepAdd;
        break;
    case Qt::Key_Right:
        main   = (orientation() == Qt::Horizontal);
        action = !invertedAppearance() ? SliderSingleStepAdd : SliderSingleStepSub;
        break;
    case Qt::Key_Up:
        main   = (orientation() == Qt::Vertical);
        action = invertedControls() ? SliderSingleStepSub : SliderSingleStepAdd;
        break;
    case Qt::Key_Down:
        main   = (orientation() == Qt::Vertical);
        action = invertedControls() ? SliderSingleStepAdd : SliderSingleStepSub;
        break;
    case Qt::Key_Home:
        main   = (qxt_d().mainControl == QxtSpanSliderPrivate::LowerHandle);
        action = SliderToMinimum;
        break;
    case Qt::Key_End:
        main   = (qxt_d().mainControl == QxtSpanSliderPrivate::UpperHandle);
        action = SliderToMaximum;
        break;
    default:
        event->ignore();
        break;
    }

    if (action)
        qxt_d().triggerAction(action, main);
}

/*!
    \reimp
 */
void QxtSpanSlider::mousePressEvent(QMouseEvent* event)
{
    if (minimum() == maximum() || (event->buttons() ^ event->button()))
    {
        event->ignore();
        return;
    }

    qxt_d().handleMousePress(event->pos(), qxt_d().upperPressed, qxt_d().upper, QxtSpanSliderPrivate::UpperHandle);
    if (qxt_d().upperPressed != QStyle::SC_SliderHandle)
        qxt_d().handleMousePress(event->pos(), qxt_d().lowerPressed, qxt_d().lower, QxtSpanSliderPrivate::LowerHandle);

    event->accept();
}

/*!
    \reimp
 */
void QxtSpanSlider::mouseMoveEvent(QMouseEvent* event)
{
    emit spanSliderMoved(qxt_d().lower, qxt_d().upper);

    if (qxt_d().lowerPressed != QStyle::SC_SliderHandle && qxt_d().upperPressed != QStyle::SC_SliderHandle)
    {
        event->ignore();
        return;
    }

    QStyleOptionSlider opt;
    qxt_d().initStyleOption(&opt);
    const int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
    int newPosition = qxt_d().pixelPosToRangeValue(qxt_d().pick(event->pos()) - qxt_d().offset);
    if (m >= 0)
    {
        const QRect r = rect().adjusted(-m, -m, m, m);
        if (!r.contains(event->pos()))
        {
            newPosition = qxt_d().position;
        }
    }

    if (qxt_d().lowerPressed == QStyle::SC_SliderHandle)
    {
        if (newPosition > qxt_d().upper)
        {
            qxt_d().swapControls();
            setUpperValue(newPosition);
        }
        else
        {
            setLowerValue(newPosition);
        }
    }
    else if (qxt_d().upperPressed == QStyle::SC_SliderHandle)
    {
        if (newPosition < qxt_d().lower)
        {
            qxt_d().swapControls();
            setLowerValue(newPosition);
        }
        else
        {
            setUpperValue(newPosition);
        }
    }
    event->accept();
}

/*!
    \reimp
 */
void QxtSpanSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QSlider::mouseReleaseEvent(event);
    qxt_d().lowerPressed = QStyle::SC_None;
    qxt_d().upperPressed = QStyle::SC_None;
    update();
}

/*!
    \reimp
 */
void QxtSpanSlider::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);

    // ticks
    QStyleOptionSlider opt;
    qxt_d().initStyleOption(&opt);
    opt.subControls = QStyle::SC_SliderTickmarks;
    painter.drawComplexControl(QStyle::CC_Slider, opt);

    // groove
    opt.sliderPosition = 0;
    opt.subControls = QStyle::SC_SliderGroove;
    painter.drawComplexControl(QStyle::CC_Slider, opt);

    // handle rects
    opt.sliderPosition = qxt_d().lower;
    const QRect lr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    const int lrv  = qxt_d().pick(lr.center());
    opt.sliderPosition = qxt_d().upper;
    const QRect ur = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    const int urv  = qxt_d().pick(ur.center());

    // span
    const int minv = qMin(lrv, urv);
    const int maxv = qMax(lrv, urv);
    const QPoint c = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this).center();
    QRect spanRect;
    if (orientation() == Qt::Horizontal)
        spanRect = QRect(QPoint(minv, c.y()-2), QPoint(maxv, c.y()+1));
    else
        spanRect = QRect(QPoint(c.x()-2, minv), QPoint(c.x()+1, maxv));
    qxt_d().drawSpan(&painter, spanRect);

    // handles
    switch (qxt_d().lastPressed) {
    case QxtSpanSliderPrivate::LowerHandle:
        qxt_d().drawHandle(&painter, QxtSpanSliderPrivate::UpperHandle);
        qxt_d().drawHandle(&painter, QxtSpanSliderPrivate::LowerHandle);
        break;
    case QxtSpanSliderPrivate::UpperHandle:
    case QxtSpanSliderPrivate::NoHandle:
    default:
        qxt_d().drawHandle(&painter, QxtSpanSliderPrivate::LowerHandle);
        qxt_d().drawHandle(&painter, QxtSpanSliderPrivate::UpperHandle);
        break;
    }
}
