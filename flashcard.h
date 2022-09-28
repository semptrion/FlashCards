#ifndef FLASHCARD_H
#define FLASHCARD_H

const int MENU_CREAR        = 1;
const int MENU_ABRIR        = 2;
const int MENU_GUARDAR      = 3;
const int MENU_GUARDAR_COMO = 4;
const int MENU_CERRAR       = 5;

const int REQ_SIN_REQUERIMIENTO = 0;
const int REQ_ESTUDIO = 1;


#include <QMainWindow>
#include <QStringListModel>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class FlashCard; }
QT_END_NAMESPACE

class FlashCard : public QMainWindow
{
    Q_OBJECT

public:
    FlashCard(QWidget *parent = nullptr);
    ~FlashCard();

private slots:
    void on_actionCrear_triggered();

    void on_actionAbrir_triggered();

    void on_actionGuardar_triggered();

    void on_actionCerrar_triggered();

    void on_actionSalir_triggered();

    void on_cards_itemChanged();

    void on_tema_textChanged();

    void on_crearFila_clicked();

    void on_cards_cellClicked(int row, int column);

    void on_avanzar_clicked();

    void on_retroceder_clicked();

    void on_siguiente_clicked();

    void on_mostrar_clicked();

    void on_actionAcerca_de_triggered();

    void on_actionGuardarComo_triggered();

    void on_estPregunta_textChanged();

    void on_estRespuesta_textChanged();

    void on_grabar_clicked();

private:
    Ui::FlashCard *ui;
    void actualizarEstado();
    void activarEdicion();
    void desactivarEdicion();
    void limpiarControles();
    QString textoALinea(QString texto);
    int requerimiento = REQ_SIN_REQUERIMIENTO;
    int filas = -1;
    int estudiar = -1;
    int evaluar = -1;
    bool mostrarRespuesta = false;
    bool archivoModificado = false;
    bool archivoNuevo = false;
    bool modoEdicion = false;
    bool modoProcesamiento = false;
    QString nombreArchivo = NULL;
    bool verificarCambios(QString confirmar);
    QString lineaATexto(QString texto);
};
#endif // FLASHCARD_H
