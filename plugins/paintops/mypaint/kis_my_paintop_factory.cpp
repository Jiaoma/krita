#include "kis_my_paintop_factory.h"

#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <kis_mypaint_brush.h>
#include <kis_my_paintop.h>
#include <kis_my_paintop_settings.h>
#include <kis_my_paintop_settings_widget.h>
#include <KisResourceServerProvider.h>
#include <kis_my_paintop_option.h>
#include <kis_icon.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <qmath.h>

class KisMyPaintOpFactory::Private {

    public:
        KoResourceServer<KisMyPaintBrush> *brushServer;
        QMap<QString, KisMyPaintBrush*> brushes;
};

KisMyPaintOpFactory::KisMyPaintOpFactory()
    : m_d(new Private)
{

    m_d->brushServer = new KoResourceServerSimpleConstruction<KisMyPaintBrush>("mypaint_brushes", "*.myb");
    m_d->brushServer->loadResources(KoResourceServerProvider::blacklistFileNames(m_d->brushServer->fileNames(), m_d->brushServer->blackListedFiles()));
}

KisMyPaintOpFactory::~KisMyPaintOpFactory() {

//    delete m_d->brushServer;
//    delete m_d;
}

KisPaintOp* KisMyPaintOpFactory::createOp(const KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image) {

    KisPaintOp* op = new KisMyPaintOp(settings, painter, node, image);

    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettingsSP KisMyPaintOpFactory::settings() {

    KisPaintOpSettingsSP settings = new KisMyPaintOpSettings();
//    settings->setModelName(m_model);
    return settings;
}

KisPaintOpConfigWidget* KisMyPaintOpFactory::createConfigWidget(QWidget* parent) {

    return new KisMyPaintOpSettingsWidget(parent);
}

QString KisMyPaintOpFactory::id() const {

    return "mypaintbrush";
}

QString KisMyPaintOpFactory::name() const {

    return "MyPaint";
}

QIcon KisMyPaintOpFactory::icon() {

    return KisIconUtils::loadIcon(id());
}

QString KisMyPaintOpFactory::category() const {

    return KisPaintOpFactory::categoryStable();
}

void KisMyPaintOpFactory::processAfterLoading() {

    KisPaintOpPresetResourceServer *paintOpServer = KisResourceServerProvider::instance()->paintOpPresetServer();

    foreach(KisMyPaintBrush* brush, m_d->brushServer->resources()) {

        QFileInfo fileInfo(brush->filename());

        KisPaintOpSettingsSP s = new KisMyPaintOpSettings();
        s->setProperty("paintop", id());
        s->setProperty("filename", brush->filename());
        s->setProperty(MYPAINT_JSON, brush->getJsonData());
        s->setProperty(MYPAINT_DIAMETER, brush->getSize());
        s->setProperty(MYPAINT_HARDNESS, brush->getHardness());
        s->setProperty(MYPAINT_OPACITY, brush->getOpacity());
        s->setProperty(MYPAINT_ERASER, brush->isEraser());
        s->setProperty("EraserMode", qRound(brush->isEraser()));

//        QJsonDocument doc = QJsonDocument::fromJson(s->getProperty(MYPAINT_JSON).toByteArray());
//        QJsonObject brush_json = doc.object();
//        QVariantMap map = brush_json.toVariantMap();
//        QVariantMap settings_map = map["settings"].toMap();
//        QVariantMap name_map = settings_map["hardness"].toMap();
//        double base_val = name_map["base_value"].toDouble();

//        name_map["base_value"] = 0.5;
//        settings_map["hardness"] = name_map;
//        map["settings"] = settings_map;

//        QJsonObject json_obj = QJsonObject::fromVariantMap(map);
//        QJsonDocument doc2(json_obj);

//        s->setProperty(MYPAINT_JSON, doc2.toJson());

        KisPaintOpPresetSP preset = new KisPaintOpPreset();
        preset->setName(fileInfo.baseName());
        preset->setSettings(s);
        KoID paintOpID(id(), name());
        preset->setPaintOp(paintOpID);        
        preset->setImage(brush->image());        
        preset->setValid(true);        

        paintOpServer->addResource(preset, false);
    }

}

#include "kis_my_paintop_factory.moc"
