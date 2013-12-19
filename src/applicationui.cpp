/*
 * Copyright (c) 2011-2013 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/cascades/Sheet>
#include <bb/cascades/ActivityIndicator>
#include <bb/cascades/Label>
#include <QtLocationSubset/QGeoSearchManager>
#include <QtLocationSubset/QGeoServiceProvider>
#include <QtLocationSubset/QGeoSearchReply>
#include <QtLocationSubset/QGeoAddress>
#include <QtLocationSubset/QGeoBoundingCircle>
#include <QtLocationSubset/QGeoPlace>
#include <QtLocationSubset/QGeoCoordinate>
#include <bb/cascades/maps/MapView>
#include <bb/cascades/maps/MapData>
#include <bb/platform/geo/GeoLocation>
#include <bb/platform/geo/Marker>

using namespace bb::cascades;
using namespace QtMobilitySubset;
using namespace bb::cascades::maps;
using namespace bb::platform::geo;

ApplicationUI::ApplicationUI(bb::cascades::Application *app) :
        QObject(app)
{
    // prepare the localization
    m_pTranslator = new QTranslator(this);
    m_pLocaleHandler = new LocaleHandler(this);

    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged()));
    // This is only available in Debug builds
    Q_ASSERT(res);
    // Since the variable is not used in the app, this is added to avoid a
    // compiler warning
    Q_UNUSED(res);

    // initial load
    onSystemLanguageChanged();

    // Create scene document from main.qml asset, the parent is set
    // to ensure the document gets destroyed properly at shut down.
    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);
    qml->setContextProperty("app", this);
    root = qml->createRootObject<AbstractPane>();
    // Set created root object as the application scene
    app->setScene(root);
}

void ApplicationUI::geoCoding(const QString &dir){
	// Initialize QGeoSearchManager
	QStringList serviceProviders = QGeoServiceProvider::availableServiceProviders();
	QGeoServiceProvider *serviceProvider;
	QGeoSearchManager *searchManager;
	QGeoAddress *address = new QGeoAddress();
	QGeoBoundingCircle *bound = new QGeoBoundingCircle();
	address->setText(dir);
	if ( serviceProviders.size() ) {
		serviceProvider = new QtMobilitySubset::QGeoServiceProvider(
						  serviceProviders.at(0) );
		searchManager = serviceProvider->searchManager();
	}
	// Request geocoding
	m_pReply = searchManager->geocode(*address,bound);
	bool finished_connected = QObject::connect(m_pReply, SIGNAL(finished()),this,SLOT(readGeocode()));

	if (finished_connected) {
		// Signal was successfully connected.
	} else {
		// Failed to connect to signal.
		// This is not normal in most cases and can be
		// a critical situation for your app! Make sure
		// you know exactly why this has happened. Add
		// some code to recover from the lost connection
		// below this line.
	}
}

void ApplicationUI::readGeocode(){
	Sheet *sheet = root->findChild<Sheet*>("mapa");
	AbstractPane *rootSheet = sheet->content();
	MapView *myMapView = rootSheet->findChild<MapView*>("mapView");
	ActivityIndicator *activity = rootSheet->findChild<ActivityIndicator*>("loading");
	QList<QGeoPlace> lugares = m_pReply->places();
	int cant =  lugares.size();
	QGeoCoordinate coor;
	if (cant>0) {
		coor = lugares.at(0).coordinate();
		myMapView->setAltitude(coor.altitude());
		myMapView->setLatitude(coor.latitude());
		myMapView->setLongitude(coor.longitude());
		GeoLocation* myLocation = new GeoLocation("id-myLocation", lugares.at(0).address().text(), Point(coor.latitude(), coor.longitude()));
		Marker myMarker;
		myMarker.setIconUri("/usr/hmi/lbs/purple_pin.png");
		myMarker.setIconSize(QSize(64, 64));
		myMarker.setLocationCoordinate(QPoint(24, 64));
		myMarker.setCaptionTailCoordinate(QPoint(24, 3));

		myLocation->setMarker(myMarker);
		myMapView->mapData()->add(myLocation);
		myMapView->setLocationOnVisible();
		activity->setVisible(false);
		myMapView->setVisible(true);
	}
	else {
		Label *labelError = rootSheet->findChild<Label*>("errorGeo");
		labelError->setVisible(true);
		activity->setVisible(false);
	}
	m_pReply->deleteLater();
}

void ApplicationUI::onSystemLanguageChanged()
{
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    // Initiate, load and install the application translation files.
    QString locale_string = QLocale().name();
    QString file_name = QString("GeoCoding_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}
