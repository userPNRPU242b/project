#include "archive.h"
#include "ui_archive.h"
#include "window_to_enter.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QHeaderView>
#include <QDate>
#include <QTimer>

archive::archive(QWidget *parent, const QString &username) :
    QDialog(parent),
    ui(new Ui::archive),
    currentUsername(username)
{
    ui->setupUi(this);

    // Настройка основного окна
    setupWindowProperties();
    setupTable();
    setupSaveButton();
    loadDataFromDatabase();
}

archive::~archive()
{
    delete ui;
}

// Настройка свойств окна
void archive::setupWindowProperties()
{
    // Установка начального размера и политики растяжения
    this->resize(900, 500);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setWindowFlags(this->windowFlags() | Qt::WindowMaximizeButtonHint);

    // Установка стилей через отдельный метод для лучшей читаемости
    applyStyles();
}

// Применение стилей к интерфейсу
void archive::applyStyles()
{
    // Устанавливаем стиль для всего приложения
    QString styleSheet = R"(
        /* Основной фон окна */
        QDialog {
            background-color: rgb(76, 177, 255);
            font-family: 'Segoe UI';
        }

        /* Прозрачный фон для дочерних виджетов */
        QDialog > QWidget {
            background: transparent;
        }

        /* Стиль кнопок */
        QPushButton {
            background-color: rgb(76, 177, 255);
            color: rgb(255, 255, 255);
            border: 2px solid rgb(255, 255, 255);
            border-radius: 10px;
            padding: 8px 12px;
            font: bold 11pt 'Segoe UI';
            min-width: 120px;
        }

        /* Состояние при наведении */
        QPushButton:hover {
            background-color: rgb(100, 190, 255);
            border: 2px solid rgb(255, 255, 255);
        }

        /* Состояние при нажатии */
        QPushButton:pressed {
            background-color: rgb(50, 140, 220);
            border: 2px solid rgb(255, 255, 255);
            padding-top: 9px;
            padding-bottom: 7px;
        }

        /* Стиль таблицы */
        QTableWidget {
            background-color: white;
            gridline-color: rgb(200, 200, 200);
            font: bold 10pt 'Segoe UI';
            color: black;
            border: 1px solid rgb(200, 200, 200);
            border-radius: 5px;
        }

        /* Стиль элементов таблицы */
        QTableWidget QTableCornerButton::section {
            background-color: rgb(76, 177, 255);
        }

        /* Стиль обычных строк */
        QTableWidget::item {
            background-color: white;
            color: black;
            border: none;
            padding: 5px;
        }

        /* Стиль при наведении */
        QTableWidget::item:hover {
            background-color: rgb(100, 190, 255);
            color: white;
        }

        /* Стиль выделенной строки */
        QTableWidget::item:selected {
            background-color: rgb(100, 190, 255);
            color: white;
        }

        /* Стиль заголовков таблицы */
        QHeaderView::section {
            background-color: rgb(76, 177, 255);
            color: white;
            padding: 5px;
            border: none;
            font: bold 11pt 'Segoe UI';
        }

        /* Стиль для полей ввода */
        QLineEdit, QTextEdit {
            background-color: white;
            color: black;
            border: 1px solid rgb(200, 200, 200);
            border-radius: 5px;
            padding: 5px;
            font: bold 10pt 'Segoe UI';
            selection-background-color: rgb(100, 190, 255);
            selection-color: white;
        }
    )";

    this->setStyleSheet(styleSheet);
}

// Настройка кнопки сохранения
void archive::setupSaveButton()
{
    QPushButton *saveButton = new QPushButton("Сохранить изменения", this);
    connect(saveButton, &QPushButton::clicked, this, &archive::saveAllChanges);
    this->layout()->addWidget(saveButton);

    // Стилизация кнопки
    saveButton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgb(76, 177, 255);"
        "   color: white;"
        "   border: 2px solid white;"
        "   border-radius: 10px;"
        "   padding: 5px;"
        "   font: bold 11pt 'Segoe UI';"
        "   min-width: 150px;"
        "   min-height: 30px;"
        "}"
        "QPushButton:hover { background-color: rgb(58, 155, 220); }"
        "QPushButton:pressed { background-color: rgb(0, 170, 255); }"
        );

    // Подсказка для пользователя
    saveButton->setToolTip("Сохранить все изменения в базе данных");
}

// Настройка таблицы данных
void archive::setupTable()
{
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels(
        QStringList() << "Дата" << "Произведенные работы" << "Часы наработки" << "Комментарии");

    // Настройка поведения таблицы
    ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    // Для столбца с датой - фиксированная ширина
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableWidget->setColumnWidth(0, 100);
    ui->tableWidget->setColumnWidth(2, 130);

    // Для столбца с часами - по содержимому
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    // Для комментариев - растягивающийся
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setAlternatingRowColors(true);

    // Разрешаем редактирование только столбца с комментариями
    ui->tableWidget->setColumnHidden(0, false); // Скрываем ID, если нужно
}

// Загрузка данных из базы данных
void archive::loadDataFromDatabase()
{
    QSqlDatabase db = QSqlDatabase::database(currentUsername);
    if (!db.isOpen()) {
        showError("База данных не открыта");
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, date, work_description, hours, comments FROM working_hours "
                  "WHERE username = ? ORDER BY date DESC, id DESC");
    query.addBindValue(currentUsername);

    if (!query.exec()) {
        showError("Ошибка загрузки данных: " + query.lastError().text());
        return;
    }

    ui->tableWidget->setRowCount(0); // Очищаем таблицу перед загрузкой

    while (query.next()) {
        addTableRow(query);
    }
}

// Добавление строки в таблицу
void archive::addTableRow(const QSqlQuery& query)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    // ID записи (скрытое поле)
    int recordId = query.value(0).toInt();

    // Дата (не редактируемая)
    addNonEditableItem(row, 0, query.value(1).toDate().toString("dd.MM.yyyy"), recordId);

    // Описание работ (не редактируемое)
    addNonEditableItem(row, 1, query.value(2).toString());

    // Часы наработки (не редактируемые)
    addNonEditableItem(row, 2, QString::number(query.value(3).toDouble(), 'f', 2));

    // Комментарии (редактируемые)
    ui->tableWidget->setItem(row, 3, new QTableWidgetItem(query.value(4).toString()));
}

// Добавление нередактируемого элемента таблицы
void archive::addNonEditableItem(int row, int column, const QString& text, const QVariant& userData)
{
    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    if (userData.isValid()) {
        item->setData(Qt::UserRole, userData);
    }
    ui->tableWidget->setItem(row, column, item);
}

// Сохранение всех изменений
void archive::saveAllChanges()
{
    QSqlDatabase db = QSqlDatabase::database(currentUsername);
    if (!db.isOpen()) {
        showError("Нет подключения к БД");
        return;
    }

    // Начинаем транзакцию для атомарности операций
    if (!db.transaction()) {
        showError("Не удалось начать транзакцию");
        return;
    }

    bool hasErrors = false;

    // Сохраняем все измененные комментарии
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        if (!saveRowChanges(row, db)) {
            hasErrors = true;
        }
    }

    // Завершаем транзакцию
    finalizeTransaction(db, hasErrors);
}

// Сохранение изменений в строке
bool archive::saveRowChanges(int row, QSqlDatabase& db)
{
    int recordId = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
    QString newComment = ui->tableWidget->item(row, 3)->text();

    QSqlQuery query(db);
    query.prepare("UPDATE working_hours SET comments = ? WHERE id = ? AND username = ?");
    query.addBindValue(newComment);
    query.addBindValue(recordId);
    query.addBindValue(currentUsername);

    if (!query.exec()) {
        qDebug() << "Ошибка сохранения строки" << row << ":" << query.lastError().text();
        return false;
    }
    return true;
}

// Завершение транзакции
void archive::finalizeTransaction(QSqlDatabase& db, bool hasErrors)
{
    if (hasErrors) {
        db.rollback();
        showWarning("Не все изменения сохранены. Проверьте данные.");
    } else {
        if (!db.commit()) {
            showError("Ошибка фиксации транзакции");
            db.rollback();
        } else {
            showInfo("Все изменения сохранены");
            highlightSavedChanges();
        }
    }
}

// Подсветка сохраненных изменений
void archive::highlightSavedChanges()
{
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        if (ui->tableWidget->item(row, 3)) {
            ui->tableWidget->item(row, 3)->setBackground(QColor(200, 255, 200));
            QTimer::singleShot(1000, [this, row]() {
                if (ui->tableWidget->item(row, 3)) {
                    ui->tableWidget->item(row, 3)->setBackground(Qt::white);
                }
            });
        }
    }
}

// Обработчик изменения ячейки таблицы
void archive::on_tableWidget_cellChanged(int row, int column)
{
    if (column != 3) return; // Реагируем только на изменения в столбце комментариев

    QSqlDatabase db = QSqlDatabase::database(currentUsername);
    if (!db.isOpen()) {
        showError("Нет подключения к БД");
        return;
    }

    int recordId = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
    QString newComment = ui->tableWidget->item(row, 3)->text();

    QSqlQuery query(db);
    query.prepare("UPDATE working_hours SET comments = ? WHERE id = ? AND username = ?");
    query.addBindValue(newComment);
    query.addBindValue(recordId);
    query.addBindValue(currentUsername);

    if (query.exec()) {
        highlightSavedRow(row);
    } else {
        showWarning("Не удалось сохранить комментарий: " + query.lastError().text());
    }
}

// Подсветка сохраненной строки
void archive::highlightSavedRow(int row)
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->item(row, 3)->setBackground(QColor(200, 255, 200));
    ui->tableWidget->blockSignals(false);

    QTimer::singleShot(1000, [this, row]() {
        if (ui->tableWidget->item(row, 3)) {
            ui->tableWidget->blockSignals(true);
            ui->tableWidget->item(row, 3)->setBackground(Qt::white);
            ui->tableWidget->blockSignals(false);
        }
    });
}

// Вспомогательные методы для отображения сообщений
void archive::showError(const QString& message) {
    QMessageBox::critical(this, "Ошибка", message);
}
void archive::showWarning(const QString& message) {
    QMessageBox::warning(this, "Внимание", message);
}
void archive::showInfo(const QString& message) {
    QMessageBox::information(this, "Информация", message);
}

// Возврат в основное окно
void archive::on_pushButton_clicked()
{
    window_to_enter *newWindow = new window_to_enter(this, currentUsername);
    newWindow->show();
    this->hide();
}
