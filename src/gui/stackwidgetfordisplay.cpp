#include "stackwidgetfordisplay.h"
#include <QLabel>
#include "pagedisplay.h"
#include "pagesynthesis.h"
#include "pagetranslation.h"
#include "pagetranscription.h"

StackWidgetForDisplay::StackWidgetForDisplay(QWidget *parent)
    : QStackedWidget(parent)
{
    _page_display = new PageDisplay(this);
    this->addWidget(_page_display);
    _page_display->setObjectName("PageForDisplay");
    _page_display->setAttribute(Qt::WA_StyledBackground, true);

    _page_transcription = new PageTranscription(this);
    this->addWidget(_page_transcription);
    _page_transcription->setObjectName("PageForTranscription");
    _page_transcription->setAttribute(Qt::WA_StyledBackground, true);

    _page_translation = new PageTranslation(this);
    this->addWidget(_page_translation);
    _page_translation->setObjectName("PageForTranslation");
    _page_translation->setAttribute(Qt::WA_StyledBackground, true);

    _page_synthesis = new PageSynthesis(this);
    this->addWidget(_page_synthesis);
    _page_synthesis->setObjectName("PageForSynthesis");
    _page_synthesis->setAttribute(Qt::WA_StyledBackground, true);

}

void StackWidgetForDisplay::slot_ChangeCurrentPage(int currentRow)
{
    this->setCurrentIndex(currentRow);
}
