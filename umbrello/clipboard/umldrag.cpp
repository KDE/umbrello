/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define FILEVERSION 5

#include "umldrag.h"
#include "../activitywidgetdata.h"
#include "../actor.h"
#include "../actorwidgetdata.h"
#include "../attribute.h"
#include "../concept.h"
#include "../conceptwidgetdata.h"
#include "../floatingtextdata.h"
#include "../messagewidgetdata.h"
#include "../notewidgetdata.h"
#include "../objectwidgetdata.h"
#include "../operation.h"
#include "../statewidgetdata.h"
#include "../umldoc.h"
#include "../umlwidgetdata.h"
#include "../usecase.h"
#include "../usecasewidgetdata.h"

class UMLDragPrivate {
public:
	UMLDragPrivate() {
		setSubType("clip1", 0);
	}
	enum { nfmt=4 };
	QCString fmt[nfmt];
	QCString subtype;
	QByteArray enc[nfmt];

	void setType(const QCString & st, int index) {
		if(index < nfmt) {
			fmt[index] = st.lower();
		}
	}

	void setSubType(const QCString & st, int index) {
		if(index < nfmt) {
			subtype = st.lower();
			fmt[index] = "application/x-uml-";
			fmt[index].append(subtype);
		}
	}
	const char * format(int i) const {
		if(i < nfmt) {
			return fmt[i];
		}
		return 0;
	}
};


UMLDrag::UMLDrag( UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
                    QWidget * dragSource /*= 0*/, const char * name /*= 0*/ ): QDragObject( dragSource, name ) {
	d = new UMLDragPrivate;
	setUMLData( Objects, UMLListViewItems );
}

UMLDrag::UMLDrag( UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems, UMLViewDataList& Diagrams,
                    QWidget * dragSource /*= 0*/, const char * name /*= 0*/ ): QDragObject( dragSource, name ) {
	d = new UMLDragPrivate;
	setUMLData( Objects, UMLListViewItems, Diagrams);
}

UMLDrag::UMLDrag( UMLListViewItemDataList& UMLListViewItems, QWidget * dragSource /*= 0*/, const char * name /*= 0*/ ):
QDragObject( dragSource, name ) {

	d = new UMLDragPrivate;
	setUMLData( UMLListViewItems );
}
UMLDrag::UMLDrag( UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems, UMLWidgetDataList& WidgetDatas,
                    AssociationWidgetDataList& AssociationDatas, QPixmap& PngImage, Uml::Diagram_Type dType, QWidget * dragSource /*= 0*/, const char * name /*= 0*/ ):
QDragObject( dragSource, name ) {
	d = new UMLDragPrivate;
	setUMLData( Objects, UMLListViewItems, WidgetDatas, AssociationDatas, PngImage, dType);
}

UMLDrag::UMLDrag( UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,  int I,
                    QWidget * /*dragSource = 0*/, const char * /*name = 0*/ ) {
	d = new UMLDragPrivate;
	setUMLData( Objects, UMLListViewItems, I);
}

UMLDrag::UMLDrag( QWidget * dragSource /*= 0*/, const char * name /*= 0*/ )   : QDragObject( dragSource, name ) {
	d = new UMLDragPrivate;
}

UMLDrag::~UMLDrag() {
	delete d;
}

void UMLDrag::setSubType( const QCString & st, int index) {
	d->setSubType(st, index);
}

void UMLDrag::setEncodedData( const QByteArray & encodedData, int index ) {
	d->enc[index] = encodedData.copy();
}

QByteArray UMLDrag::encodedData(const char* m) const {
	QString str(m);
	for( int i = 0; i < 4; i++) {
		if ( !qstricmp(m,d->fmt[i]) ) {
			return d->enc[i];
		}
	}

	return QByteArray();
}

const char * UMLDrag::format(int i) const {

	char* result = (char*)d->format(i);

	return result;
}

void UMLDrag::setUMLData( UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems ) {
	long l_size = 0;

	setSubType("clip1", 0);
	UMLObjectListIt object_it( Objects );
	UMLObject *obj;
	while ( (obj=object_it.current()) != 0 ) {
		++object_it;
		l_size += obj->getClipSizeOf();
	}

	l_size += sizeof(Objects.count());
	l_size += sizeof(UMLListViewItems.count());

	UMLListViewItemDataListIt item_it( UMLListViewItems );
	UMLListViewItemData * item;
	while ( (item=item_it.current()) != 0 ) {
		++item_it;
		l_size += item->getClipSizeOf();
	}
	char *data = new char[l_size];
	QByteArray clipdata;
	clipdata.setRawData(data, l_size);
	QDataStream clipstream( clipdata, IO_WriteOnly );
	clipstream << Objects.count();

	UMLObjectListIt object_it2( Objects );
	while ( (obj=object_it2.current()) != 0 ) {
		++object_it2;
		obj->serialize(&clipstream, true, FILEVERSION);
	}

	clipstream << UMLListViewItems.count();
	UMLListViewItemDataListIt item_it2( UMLListViewItems );
	while ( (item=item_it2.current()) != 0 ) {
		++item_it2;
		item->clipSerialize(&clipstream, true);
	}

	setEncodedData(clipdata, 0);
}

void UMLDrag::setUMLData( UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems, UMLViewDataList& Diagrams ) {
	long l_size = 0;

	setSubType("clip2", 0);
	UMLObjectListIt object_it( Objects );
	UMLObject *obj;
	while ( (obj=object_it.current()) != 0 ) {
		++object_it;
		l_size += obj->getClipSizeOf();
	}



	UMLViewDataListIt diagram_it( Diagrams );
	UMLViewData * view;

	while ( (view=diagram_it.current()) != 0 ) {
		++diagram_it;
		l_size += view->getClipSizeOf();
	}
	l_size += sizeof(Objects.count());
	l_size += sizeof(Diagrams.count());
	l_size += sizeof(UMLListViewItems.count());

	UMLListViewItemDataListIt item_it( UMLListViewItems );
	UMLListViewItemData * item;
	while ( (item=item_it.current()) != 0 ) {
		++item_it;
		l_size += item->getClipSizeOf();
	}

	char *data = new char[l_size];
	QByteArray clipdata;
	clipdata.setRawData(data, l_size);
	QDataStream clipstream( clipdata, IO_WriteOnly );
	clipstream << Objects.count();

	UMLObjectListIt object_it2( Objects );
	while ( (obj=object_it2.current()) != 0 ) {
		++object_it2;
		obj->serialize(&clipstream, true, FILEVERSION);
	}

	clipstream << Diagrams.count();
	UMLViewDataListIt diagram_it2( Diagrams );
	while ( (view=diagram_it2.current()) != 0 ) {
		++diagram_it2;
		view->serialize(&clipstream, true, FILEVERSION);
	}

	clipstream << UMLListViewItems.count();
	UMLListViewItemDataListIt item_it2( UMLListViewItems );

	while ( (item=item_it2.current()) != 0 ) {
		++item_it2;
		item->clipSerialize(&clipstream, true);
	}

	setEncodedData(clipdata, 0);
}

void UMLDrag::setUMLData( UMLListViewItemDataList& UMLListViewItems ) {
	long l_size = 0;

	setSubType("clip3", 0);
	l_size += sizeof(UMLListViewItems.count());

	UMLListViewItemDataListIt item_it( UMLListViewItems );
	UMLListViewItemData * item;
	while ( (item=item_it.current()) != 0 ) {
		++item_it;
		l_size += item->getClipSizeOf();
	}

	char *data = new char[l_size];
	QByteArray clipdata;
	clipdata.setRawData(data, l_size);
	QDataStream clipstream( clipdata, IO_WriteOnly );

	clipstream << UMLListViewItems.count();

	UMLListViewItemDataListIt item_it2( UMLListViewItems );
	while ( (item=item_it2.current()) != 0 ) {
		++item_it2;
		item->clipSerialize(&clipstream, true);
	}


	setEncodedData(clipdata, 0);

}

void UMLDrag::setUMLData( UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
                           UMLWidgetDataList& WidgetDatas,	AssociationWidgetDataList& AssociationDatas, QPixmap& PngImage, Uml::Diagram_Type dType ) {
	long l_size = 0;

	setSubType("clip4", 0);
	UMLObjectListIt object_it( Objects );
	UMLObject *obj;
	while ( (obj=object_it.current()) != 0 )
	{
		++object_it;
		l_size += obj->getClipSizeOf();
	}

	UMLWidgetDataListIt widgetdata_it( WidgetDatas );
	UMLWidgetData *widgetdata;
	QString stype;
	Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string
	//bool save = true;

	while ( (widgetdata=widgetdata_it.current()) != 0 ) {
		++widgetdata_it;
		switch(widgetdata -> getType()) {
			case Uml::wt_Actor:
				stype = "ACTOR";
				break;
			case Uml::wt_UseCase:
				stype = "USECASE";
				break;
			case Uml::wt_Class:
				stype = "CONCEPT";
				break;
			case Uml::wt_Object:
				stype = "OBJECT";
				break;
			case Uml::wt_Note:
				stype = "NOTE";
				break;

			case Uml::wt_Text:
				stype = "FLOATTEXT";
				break;
			case Uml::wt_Message:
				stype = "MESSAGEW";
				break;
			case Uml::wt_State:
				stype = "STATE";
				break;
			case Uml::wt_Activity:
				stype = "ACTIVITY";
				break;
			default:
				continue;
				break;
		}//end switch
		if ( !stype.length() ) //We assume we are working with QT 2.1.x or superior, that means
			//if unicode returns a null pointer then the serialization process of the QString object
			//will write a null marker 0xffffff, see QString::operator<< implementation
		{
			l_size += sizeof(tmp);
		} else {
			l_size += (stype.length()*sizeof(QChar));
		}
	}
	AssociationWidgetDataListIt assoc_it( AssociationDatas );
	AssociationWidgetData *assoc;
	while ( (assoc=assoc_it.current()) != 0 ) {
		++assoc_it;
		l_size += assoc->getClipSizeOf();
	}

	l_size += sizeof(Objects.count());
	l_size += sizeof(WidgetDatas.count());
	l_size += sizeof(AssociationDatas.count());
	l_size += sizeof(UMLListViewItems.count());

	UMLListViewItemDataListIt item_it( UMLListViewItems );
	UMLListViewItemData * item;
	while ( (item=item_it.current()) != 0 ) {
		++item_it;
		l_size += item->getClipSizeOf();
	}
	int diagramType = (int)dType;
	l_size += sizeof(diagramType);
	char *data = new char[l_size];
	QByteArray clipdata;
	clipdata.setRawData(data, l_size);
	QDataStream clipstream( clipdata, IO_WriteOnly );
	clipstream << diagramType;
	clipstream << Objects.count();

	UMLObjectListIt object_it2( Objects );
	while ( (obj=object_it2.current()) != 0 ) {
		++object_it2;

		obj->serialize(&clipstream, true, FILEVERSION);
	}

	clipstream << UMLListViewItems.count();
	UMLListViewItemDataListIt item_it2( UMLListViewItems );
	while ( (item=item_it2.current()) != 0 ) {
		++item_it2;
		item->clipSerialize(&clipstream, true);
	}

	clipstream << WidgetDatas.count();
	UMLWidgetDataListIt widgetdata_it2( WidgetDatas );
	while ( (widgetdata=widgetdata_it2.current()) != 0 ) {
		++widgetdata_it2;
		switch(widgetdata -> getType()) {
			case Uml::wt_Actor:
				stype = "ACTOR";
				break;
			case Uml::wt_UseCase:
				stype = "USECASE";
				break;
			case Uml::wt_Class:
				stype = "CONCEPT";
				break;
			case Uml::wt_Object:
				stype = "OBJECT";

				break;
			case Uml::wt_Note:
				stype = "NOTE";
				break;
			case Uml::wt_Text:
				stype = "FLOATTEXT";
				break;
			case Uml::wt_Message:
				stype = "MESSAGEW";
				break;
			case Uml::wt_State:
				stype = "STATE";
				break;
			case Uml::wt_Activity:
				stype = "ACTIVITY";
				break;
			default:
				return;
				break;
		}//end switch
		clipstream << stype;

		widgetdata->serialize(&clipstream, true, FILEVERSION);
	}

	clipstream << AssociationDatas.count();
	AssociationWidgetDataListIt assoc_it2( AssociationDatas );
	while ( (assoc=assoc_it2.current()) != 0 ) {
		++assoc_it2;
		assoc->serialize(&clipstream, true, FILEVERSION);

	}

	setEncodedData(clipdata, 0);

	d->setType("image/PNG", 1);
	l_size = (PngImage.convertToImage()).numBytes();
	char *data2 = new char[l_size];
	QByteArray clipdata2;
	clipdata2.setRawData(data2, l_size);
	QDataStream clipstream2( clipdata2, IO_WriteOnly );

	clipstream2 << PngImage;
	setEncodedData(clipdata2, 1);
}

void UMLDrag::setUMLData( UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems, int ) {
	long l_size = 0;

	setSubType("clip5", 0);
	UMLObjectListIt object_it( Objects );
	UMLObject *obj;



	while ( (obj=object_it.current()) != 0 ) {
		++object_it;
		l_size += obj->getClipSizeOf();
	}

	l_size += sizeof(Objects.count());
	l_size += sizeof(UMLListViewItems.count());

	UMLListViewItemDataListIt item_it( UMLListViewItems );
	UMLListViewItemData * item;
	while ( (item=item_it.current()) != 0 ) {
		++item_it;
		l_size += item->getClipSizeOf();
	}

	char *data = new char[l_size];
	QByteArray clipdata;
	clipdata.setRawData(data, l_size);
	QDataStream clipstream( clipdata, IO_WriteOnly );
	clipstream << Objects.count();


	UMLObjectListIt object_it2( Objects );
	while ( (obj=object_it2.current()) != 0 ) {
		++object_it2;
		obj->serialize(&clipstream, true, FILEVERSION);
	}

	clipstream << UMLListViewItems.count();
	UMLListViewItemDataListIt item_it2( UMLListViewItems );
	while ( (item=item_it2.current()) != 0 ) {
		++item_it2;
		item->clipSerialize(&clipstream, true);
	}


	setEncodedData(clipdata, 0);
}

bool UMLDrag::canDecode( const QMimeSource* e ) {
	const char* f;
	for (int i=0; (f=e->format(i)); i++) {
		if ( 0==qstrnicmp(f,"application/x-uml-clip", 22) ) {

			//Need to test for clip1, clip2, clip3, clip4 or clip5 (the only valid clip types)
			return TRUE;
		}
	}

	return 0;
}

bool UMLDrag::decode( const QMimeSource* e, UMLObjectList& Objects,
                       UMLListViewItemDataList& UMLListViewItems, UMLDoc* Doc) {
	if ( ! e->provides( "application/x-uml-clip1" ) ) {
		return FALSE;
	}
	QByteArray payload = e->encodedData( "application/x-uml-clip1" );
	if ( payload.size() ) {
		QDataStream clipdata(payload, IO_ReadOnly);
		int count, type;
		UMLObject* object;
		bool status = true;

		clipdata >> count;

		for(int i=0;i<count;i++) {
			clipdata >> type;
			if(type == Uml::ot_Actor) {
				UMLActor * a = new UMLActor(Doc);
				object = (UMLObject *)a;
			} else if(type == Uml::ot_UseCase)
			{
				UMLUseCase * uc = new UMLUseCase(Doc);
				object = (UMLObject *)uc;
			} else if(type == Uml::ot_Concept) {
				UMLConcept * c = new UMLConcept(Doc);
				object = (UMLObject *)c;
			} else {
				return false;
			}
			status = object -> serialize(&clipdata, false, FILEVERSION);
			if(!status) {
				return status;
			}
			Objects.append(object);
		}//end for i

		clipdata >> count;
		UMLListViewItemData* itemdata = 0;
		for(int i = 0; i < count; i++)
		{
			itemdata = new UMLListViewItemData();
			itemdata->clipSerialize(&clipdata, false);
			UMLListViewItems.append(itemdata);
		}
		return true;
	}

	return false;
}

bool UMLDrag::decode( const QMimeSource* e, UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
                       UMLViewDataList& Diagrams, UMLDoc* Doc ) {
	if ( ! e->provides( "application/x-uml-clip2" ) ) {
		return FALSE;
	}
	QByteArray payload = e->encodedData( "application/x-uml-clip2" );
	if ( payload.size() )
	{

		QDataStream clipdata(payload, IO_ReadOnly);
		int count, type;
		UMLObject* object;
		bool status = true;

		clipdata >> count;

		for(int i=0;i<count;i++) {
			clipdata >> type;
			if(type == Uml::ot_Actor) {
				UMLActor * a = new UMLActor(Doc);
				object = (UMLObject *)a;
			} else if(type == Uml::ot_UseCase) {
				UMLUseCase * uc = new UMLUseCase(Doc);
				object = (UMLObject *)uc;
			} else if(type == Uml::ot_Concept) {
				UMLConcept * c = new UMLConcept(Doc);
				object = (UMLObject *)c;
			} else {
				return false;
			}
			status = object -> serialize(&clipdata, false, FILEVERSION);
			if(!status) {
				return status;
			}
			Objects.append(object);
		}//end for i


		clipdata >> count;
		UMLViewData * view = 0;
		for(int i = 0; i < count; i++) {
			view = new UMLViewData();
			view->serialize(&clipdata, false, FILEVERSION);
			Diagrams.append(view);
		}
		clipdata >> count;

		UMLListViewItemData* itemdata = 0;
		for(int i = 0; i < count; i++) {
			itemdata = new UMLListViewItemData();
			itemdata->clipSerialize(&clipdata, false);
			UMLListViewItems.append(itemdata);
		}
		return true;
	}

	return false;
}

bool UMLDrag::decode( const QMimeSource* e, UMLListViewItemDataList& UMLListViewItems) {
	if ( ! e->provides( "application/x-uml-clip3" ) ) {
		return false;
	}
	QByteArray payload = e->encodedData( "application/x-uml-clip3" );
	if ( payload.size() ) {
		QDataStream clipdata(payload, IO_ReadOnly);
		int count;
		clipdata >> count;
		UMLListViewItemData* itemdata = 0;
		for(int i = 0; i < count; i++) {
			itemdata = new UMLListViewItemData();
			if(!	itemdata->clipSerialize(&clipdata, false)) {
				return false;
			}
			UMLListViewItems.append(itemdata);
		}

		return true;
	}
	return false;
}

bool UMLDrag::decode( const QMimeSource* e, UMLObjectList& Objects, UMLListViewItemDataList& UMLListViewItems,
                       UMLWidgetDataList& WidgetDatas, AssociationWidgetDataList& AssociationDatas, Uml::Diagram_Type & dType, UMLDoc* Doc ) {
	if ( ! e->provides( "application/x-uml-clip4" ) ) {
		return FALSE;
	}
	QByteArray payload = e->encodedData( "application/x-uml-clip4" );
	if ( payload.size() ) {
		QDataStream clipdata(payload, IO_ReadOnly);
		int count, type, diagramType;
		UMLObject* object;
		bool status = true;

		clipdata >> diagramType >> count;

		dType = (Uml::Diagram_Type)diagramType;

		for(int i=0;i<count;i++) {
			clipdata >> type;


			if(type == Uml::ot_Actor) {
				UMLActor * a = new UMLActor(Doc);
				object = (UMLObject *)a;
			} else if(type == Uml::ot_UseCase) {
				UMLUseCase * uc = new UMLUseCase(Doc);
				object = (UMLObject *)uc;
			} else if(type == Uml::ot_Concept) {
				UMLConcept * c = new UMLConcept(Doc);
				object = (UMLObject *)c;
			} else {
				return false;
			}
			status = object -> serialize(&clipdata, false, FILEVERSION);
			if(!status) {
				return status;
			}
			Objects.append(object);
		}//end for i

		clipdata >> count;
		UMLListViewItemData* itemdata = 0;
		for(int i = 0; i < count; i++) {
			itemdata = new UMLListViewItemData();
			itemdata->clipSerialize(&clipdata, false);
			UMLListViewItems.append(itemdata);
		}

		clipdata >> count;
		UMLWidgetData* widgetdata = 0;
		QString stype;
		SettingsDlg::OptionState optionState;
		optionState.uiState.lineColor = QColor("red");
		optionState.uiState.fillColor = QColor(255, 255, 192);
		for(int i = 0; i < count; i++) {
			clipdata >> stype;
			if(stype == "ACTOR") {
				ActorWidgetData * a = new ActorWidgetData(optionState);
				widgetdata = (UMLWidgetData *)a;
			}//end actor
			else if(stype == "USECASE") {
				UseCaseWidgetData * uc = new UseCaseWidgetData(optionState);
				widgetdata = (UMLWidgetData *)uc;
			}//end if usecase
			else if(stype == "CONCEPT") {
				ConceptWidgetData * c = new ConceptWidgetData(optionState);
				widgetdata = (UMLWidgetData *)c;
			}//end if concept
			else if(stype == "FLOATTEXT") {
				FloatingTextData * ft = new FloatingTextData();
				widgetdata = (UMLWidgetData *)ft;
			}//end floattext
			else if(stype == "NOTE") {
				NoteWidgetData * n = new NoteWidgetData(optionState);
				widgetdata = (UMLWidgetData *)n;
			}//end note
			else if(stype == "OBJECT") {
				ObjectWidgetData *ow = new ObjectWidgetData(optionState);
				widgetdata = (UMLWidgetData *)ow;
			}//end object
			else if(stype == "MESSAGEW") {
				MessageWidgetData *mw = new MessageWidgetData(optionState);
				widgetdata = (UMLWidgetData*)mw;
			} else if( stype == "STATE" ) {
				StateWidgetData * sw = new StateWidgetData(optionState);
				widgetdata = (UMLWidgetData *)sw;
			} else if( stype == "ACTIVITY" ) {
				ActivityWidgetData * aw = new ActivityWidgetData(optionState);
				widgetdata = (UMLWidgetData *)aw;
			} else {
				return false;
			}
			widgetdata->serialize(&clipdata, false, FILEVERSION);
			WidgetDatas.append(widgetdata);

		}


		clipdata >> count;
		AssociationWidgetData* assoc = 0;
		for(int i = 0; i < count; i++) {

			assoc = new AssociationWidgetData();
			assoc->serialize(&clipdata, false, FILEVERSION);
			AssociationDatas.append(assoc);
		}

		return true;
	}


	return false;
}

bool UMLDrag::decode( const QMimeSource* e, UMLObjectList& Objects,
                       UMLListViewItemDataList& UMLListViewItems, UMLDoc* Doc, int) {
	if ( ! e->provides( "application/x-uml-clip5" ) ) {
		return FALSE;
	}

	QByteArray payload = e->encodedData( "application/x-uml-clip5" );
	if ( payload.size() ) {
		QDataStream clipdata(payload, IO_ReadOnly);
		int count;

		Uml::UMLObject_Type type = Uml::ot_UMLObject;
		UMLObject* object;
		bool status = true;

		clipdata >> count;

		for(int i=0;i<count;i++) {
			int t;
			clipdata >> t;
			type = (Uml::UMLObject_Type)t;
			if(type == Uml::ot_Attribute) {
				UMLAttribute * a = new UMLAttribute(Doc);
				object = (UMLObject *)a;
			} else if(type == Uml::ot_Operation) {
				UMLOperation * op = new UMLOperation(Doc);
				object = (UMLObject *)op;
			} else {
				return false;
			}
			status = object -> serialize(&clipdata, false, FILEVERSION);
			if(!status) {
				return status;
			}
			Objects.append(object);
		}//end for i

		clipdata >> count;

		UMLListViewItemData* itemdata = 0;
		for(int i = 0; i < count; i++) {
			itemdata = new UMLListViewItemData();
			itemdata->clipSerialize(&clipdata, false);
			UMLListViewItems.append(itemdata);
		}
		return true;
	}

	return false;
}

int UMLDrag::getCodingType(const QMimeSource* e) {
	int result = 0;
	if (e->provides( "application/x-uml-clip1" ) ) {
		result = 1;
	}
	if (e->provides( "application/x-uml-clip2" ) ) {
		result = 2;
	}
	if (e->provides( "application/x-uml-clip3" ) ) {
		result = 3;
	}
	if (e->provides( "application/x-uml-clip4" ) ) {
		result = 4;
	}
	if (e->provides( "application/x-uml-clip5" ) ) {
		result = 5;
	}

	return result;
}
