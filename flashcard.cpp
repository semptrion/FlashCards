#include "flashcard.h"
#include "ui_flashcard.h"
#include "QFileDialog"
#include "QFile"
#include "QListWidget"
#include "QMessageBox"
#include "QDebug"
#include <QScreen>

FlashCard::FlashCard(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FlashCard)
{
    ui->setupUi(this);
    move(screen()->geometry().center() - frameGeometry().center());
    ui->cards->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->cards->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->cards->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed);
    ui->cards->setColumnWidth(2, 16);
}

FlashCard::~FlashCard()
{
    delete ui;
}

void FlashCard::activarEdicion() {
    ui->tema->setEnabled( true );
    ui->tabs->setEnabled( true );
    evaluar  = -1;
    estudiar = 0;
    filas    = ui->cards->rowCount();
    modoEdicion = true;
    mostrarRespuesta = false;
}

void FlashCard::desactivarEdicion() {
    ui->tema->setEnabled( false );
    ui->tabs->setEnabled( false );
    evaluar  = -1;
    estudiar = -1;
    filas    = -1;
    modoEdicion = false;
    mostrarRespuesta = false;
}

void FlashCard::limpiarControles() {
    modoProcesamiento = true;
    filas    = -1;
    ui->tema->setText( QString( "" ) );
    ui->cards->setRowCount( 0 );
    ui->evlPregunta->setText( QString( "" ) );
    ui->evlRespuesta->setText( QString( "" ) );
    modoProcesamiento = false;
}

bool FlashCard::verificarCambios( QString confirmar ) {
    if ( archivoModificado ) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
                    this,
                    "Existen cambios sin grabar",
                    "Existen cambios que no han sido grabados " + confirmar,
                    QMessageBox::Save | QMessageBox::Discard
                    | QMessageBox::Cancel);
        if (reply == QMessageBox::Cancel) {
            return false;
        }
        if ( reply == QMessageBox::Save )
            on_actionGuardar_triggered();
    }
    return true;
}

void FlashCard::actualizarEstado() {
    QString mensaje;
    bool existeArchivo = nombreArchivo.length() > 0;
    ui->menuArchivo->actions()
        .at( MENU_GUARDAR )->setEnabled( archivoModificado );
    ui->grabar->setEnabled( archivoModificado );
    ui->menuArchivo->actions()
        .at( MENU_GUARDAR_COMO )->setEnabled( existeArchivo );
    ui->menuArchivo->actions()
        .at( MENU_CERRAR )->setEnabled( modoEdicion );
    mensaje = "Flash Cards";
    if ( modoEdicion ) {
        mensaje = ( archivoModificado ? "  *" : "   " )
            + ( existeArchivo ? nombreArchivo: "*Archivo nuevo**" );
        if ( filas >= 0 ) {
            if ( evaluar != -1 ) {
                ui->evlPregunta->setText(
                    lineaATexto( ui->cards->item( evaluar, 0 )->text() ) );
                ui->evlRespuesta->setText(
                    mostrarRespuesta
                    ? lineaATexto( ui->cards->item( evaluar, 1 )->text() )
                    : "..." );
            }
            if ( estudiar >= filas ) estudiar = filas - 1;
            if ( estudiar < 0 ) estudiar = 0;
            ui->retroceder->setEnabled( estudiar > 0 );
            ui->avanzar->setEnabled( estudiar < filas - 1 );
            ui->progreso->setValue(
                filas > 0 ? ( ( estudiar + 1 ) * 100 ) / filas : 100 );
            if ( requerimiento != REQ_ESTUDIO ) {
                ui->estPregunta->setPlainText(
                    lineaATexto( ui->cards->item( estudiar, 0 )->text() ) );
                ui->estRespuesta->setPlainText(
                    lineaATexto( ui->cards->item( estudiar, 1 )->text() ) );
            }
            mensaje += "      ";
            if ( filas == 0 ) mensaje += "Sin preguntas";
            else if ( filas == 1 ) mensaje += "Una pregunta";
            else mensaje += QString::number( filas ) + " preguntas";
        }
    }
    ui->statusbar->showMessage( mensaje );
    requerimiento = REQ_SIN_REQUERIMIENTO;
}

QString FlashCard::textoALinea( QString texto ) {
    return texto.remove(QRegExp("[\\n ]*$"))
        .remove(QRegExp("[\\t\\r]")).replace(QString("\n"), QString("¶"));
}

QString FlashCard::lineaATexto( QString texto ) {
    return texto.replace(QString("¶"), QString("\n"));
}

void FlashCard::on_actionCrear_triggered()
{
    if ( !verificarCambios( "¿Realmente quiere crear un nuevo archivo?" ) )
        return;
    nombreArchivo = "";
    limpiarControles();
    modoProcesamiento = true; // USado para que no active la acción Grabar
    on_crearFila_clicked();
    modoProcesamiento = false; // USado para que no active la acción Grabar    
    activarEdicion();
}

void FlashCard::on_crearFila_clicked()
{
    if ( filas < 0 ) filas = 0;
    ui->cards->insertRow( filas );
    QTableWidgetItem *icon_item = new QTableWidgetItem;
    QIcon icon(":/images/delete.svg");
    icon_item->setIcon(icon);
    ui->cards->setItem( filas, 0, new QTableWidgetItem( "*Pregunta*" ) );
    ui->cards->setItem( filas, 1, new QTableWidgetItem( "*Respuesta*" ) );
    ui->cards->setItem( filas, 2, icon_item);
    filas++;
    archivoModificado = true;
    actualizarEstado();
}

void FlashCard::on_actionAbrir_triggered()
{
    if ( !verificarCambios( "¿Realmente quiere abrir un archivo?" ) ) return;
    nombreArchivo = QFileDialog::getOpenFileName(
                this,
                tr( "Abrir archivo de tarjetas" ),
                "..",
                tr("Archivo fcf (*.fcf)" ) );
    if ( nombreArchivo == "" ) return;
    QFile fcFile;
    fcFile.setFileName( nombreArchivo );
    if ( !fcFile.open(QIODevice::ReadOnly) ) {
        QMessageBox::information(0,"Error",fcFile.errorString());
        nombreArchivo = "";
        return;
    }
    limpiarControles();
    QString linea, pregunta, respuesta;
    modoProcesamiento = true; // USado para que no active la acción Grabar
    filas = -1;
    while( ( linea = fcFile.readLine() ) != NULL ) {
        if ( filas == -1 ) {
            if ( linea.left( 6 ).compare( "Tema::", Qt::CaseSensitive ) == 0 ) {
                ui->tema->setText( linea.remove(0,6).remove(QRegExp("[\\n\\t\\r]")));
                filas++;
                continue;
            } else {
                QMessageBox::information(0,"Error","El archivo no es de Flash Cards" );
                return;
            }
        }
        if ( linea.left( 10 ).compare( "Pregunta::" ) == 0 ) {
            pregunta = textoALinea(linea.remove( 0, 10 ));
        }
        if ( linea.left( 11 ).compare( "Respuesta::" ) == 0 ) {
            respuesta = textoALinea(linea.remove( 0, 11 ));
            ui->cards->insertRow( filas );
            ui->cards->setItem( filas, 0, new QTableWidgetItem( pregunta ) );
            ui->cards->setItem( filas, 1, new QTableWidgetItem( respuesta) );
            QTableWidgetItem *icon_item = new QTableWidgetItem;
            QIcon icon(":/images/delete.svg");
            icon_item->setIcon(icon);
            ui->cards->setItem(filas, 2, icon_item);
            filas++;
        }
    }
    fcFile.close();
    modoProcesamiento = false;
    archivoModificado = false;
    activarEdicion();
    actualizarEstado();
}

void FlashCard::on_actionGuardar_triggered()
{
    if ( nombreArchivo == "" ) {
        QString nuevoNombre = QFileDialog::getSaveFileName(
            this,
            tr("Guardar como..."),
            "..",
            tr("Archivo fcf (*.fcf)") );
        if ( nuevoNombre == "" ) return;
        if ( nuevoNombre.right(4).compare( ".fcf" ) != 0 ) nuevoNombre += ".fcf";
        nombreArchivo = nuevoNombre;
    }
    QString salida = "Tema::"+ ui->tema->text()+"\n";
    for ( int i = 0; i< ui->cards->rowCount(); ++i ) {
        salida += "Pregunta::" + ui->cards->item(i, 0)->text() + "\n"
                + "Respuesta::" + ui->cards->item(i, 1)->text() + "\n";
    }
    QFile fcFile;
    fcFile.setFileName( nombreArchivo );
    fcFile.open(QIODevice::WriteOnly);
    fcFile.write(salida.toUtf8());
    fcFile.close();
    archivoModificado = false;
    actualizarEstado();
    QMessageBox::information(0, "Datos grabados","El archivo de Flash Cards ha sido actualizado");
}

void FlashCard::on_actionGuardarComo_triggered()
{
    QString nuevoNombre = QFileDialog::getSaveFileName(
                this,
                tr("Guardar como..."),
                "..",
                tr("Archivo fcf (*.fcf)") );
    if ( nuevoNombre == "" ) return;
    if ( nuevoNombre.right(4).compare( ".fcf" ) != 0 ) nuevoNombre += ".fcf";
    nombreArchivo = nuevoNombre;
    on_actionGuardar_triggered();
}

void FlashCard::on_actionCerrar_triggered()
{
    if ( !verificarCambios( "¿Realmente quiere cerrar el archivo?" ) ) return;
    archivoModificado = false;
    nombreArchivo = "";
    desactivarEdicion();
    actualizarEstado();
}

void FlashCard::on_actionSalir_triggered()
{
    if ( !verificarCambios( "¿Realmente quiere salir?" ) ) return;
    close();
}

void FlashCard::on_tema_textChanged()
{
    on_cards_itemChanged();
}

void FlashCard::on_cards_itemChanged()
{
    if ( modoProcesamiento ) return;
    archivoModificado = true;
    actualizarEstado();
}

void FlashCard::on_cards_cellClicked(int row, int column)
{
    if ( column < 2 ) return;
    if( QMessageBox::question(
                this, "Eliminar pregunta",
                "La pregunta será eliminada ¿Desea continuar?",
                QMessageBox::No|QMessageBox::Yes) == QMessageBox::No ) return;
    ui->cards->removeRow(row);
    filas--;
    actualizarEstado();
}

void FlashCard::on_avanzar_clicked()
{
    estudiar++;
    actualizarEstado();
}

void FlashCard::on_retroceder_clicked()
{
    estudiar--;
    actualizarEstado();
}

/* Mostrar una tarjeta al azar */
void FlashCard::on_siguiente_clicked()
{
    int anteriorEvaluar = evaluar;
    int vuelta = 0;
    do {
        evaluar = rand() % (filas);
        vuelta++;
    } while( evaluar == anteriorEvaluar && vuelta < 5 );
    mostrarRespuesta = false;
    actualizarEstado();
}

/* Mostrar la respuesta */
void FlashCard::on_mostrar_clicked()
{
    mostrarRespuesta = true;
    actualizarEstado();
}

void FlashCard::on_actionAcerca_de_triggered()
{
    QMessageBox::information(
            this,
            tr("Tarjetas de estudio - Flash Cards"),
            tr("Proyecto desarrollado por Jaime Salamanca Mazuelo\npara la materia de Programación III\na cargo del Ing. Rodmy Orellana Illanes\n\nCarrera de Ingeniería de Sistemas\nFacultad de Ingeniería\nUniversidad Privada Domingo Savio") );
}


void FlashCard::on_estPregunta_textChanged()
{
    QString anteriorPregunta = ui->cards->item(estudiar, 0)->text();
    QString nuevaPregunta = textoALinea( ui->estPregunta->toPlainText() );
    if ( anteriorPregunta.compare(nuevaPregunta) == 0 ) return;
    requerimiento = REQ_ESTUDIO;
    ui->cards->setItem( estudiar, 0, new QTableWidgetItem( nuevaPregunta ) );
}

void FlashCard::on_estRespuesta_textChanged()
{
    QString anteriorRespuesta = ui->cards->item(estudiar, 1)->text();
    QString nuevaRespuesta = textoALinea( ui->estRespuesta->toPlainText());
    if ( anteriorRespuesta.compare(nuevaRespuesta) == 0 ) return;
    requerimiento = REQ_ESTUDIO;
    ui->cards->setItem( estudiar, 1, new QTableWidgetItem( nuevaRespuesta ) );
}

void FlashCard::on_grabar_clicked()
{
    on_actionGuardar_triggered();
}
