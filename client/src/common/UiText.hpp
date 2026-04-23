#pragma once

#include <QString>

namespace currency::client::common::UiText {

inline QString text(const char8_t* value) {
    return QString::fromUtf8(reinterpret_cast<const char*>(value));
}

inline QString appWindowTitle() { return text(u8"Currency System"); }
inline QString readyMessage() { return text(u8"Готово к работе"); }
inline QString connectionActive() { return text(u8"Соединение активно"); }
inline QString noConnection() { return text(u8"Нет соединения"); }
inline QString connected() { return text(u8"Подключено"); }
inline QString connectionRequiredMessage() { return text(u8"Сначала подключитесь к серверу и загрузите каталог валют."); }
inline QString connectionMissingDetails() { return text(u8"Откройте настройки, чтобы проверить подключение и загрузить каталог валют."); }
inline QString connectionStatusTitle() { return text(u8"Подключение к серверу"); }
inline QString serverStatusTitle() { return text(u8"Статус сервера"); }
inline QString sectionsTitle() { return text(u8"Разделы"); }
inline QString activeSourcesTitle() { return text(u8"Активные источники"); }
inline QString noSourcesSelected() { return text(u8"Источники пока не выбраны."); }
inline QString defaultDisconnectedState() { return text(u8"Соединение с сервером отсутствует."); }
inline QString defaultConnectedState() { return text(u8"Подключение к серверу установлено."); }
inline QString providersMustBeSelected() { return text(u8"Выберите хотя бы один источник данных перед запуском."); }
inline QString chooseProviderOnHomePage() { return text(u8"На главной странице выберите хотя бы один провайдер."); }
inline QString conversionIdleMessage() { return text(u8"Выберите источник, пару валют и сумму для конвертации."); }
inline QString conversionNeedsSourceMessage() { return text(u8"Подключитесь к серверу и выберите хотя бы один источник, чтобы включить конвертацию."); }
inline QString conversionLoadedMessage() { return text(u8"Результат конвертации получен с сервера."); }
inline QString ratesLoadedMessage() { return text(u8"Последние курсы успешно загружены."); }
inline QString historyLoadedMessage() { return text(u8"Исторические данные успешно загружены."); }
inline QString dashboardRatesLoadedMessage() { return text(u8"Котировки успешно обновлены."); }
inline QString dashboardHistoryLoadedMessage() { return text(u8"История успешно загружена."); }
inline QString aggregationLoadedMessage() { return text(u8"Котировки успешно загружены с сервера."); }
inline QString busyLoading() { return text(u8"Загрузка..."); }
inline QString busyRefreshing() { return text(u8"Обновление..."); }
inline QString busyExporting() { return text(u8"Экспорт..."); }
inline QString busySaving() { return text(u8"Сохранение..."); }
inline QString busyChecking() { return text(u8"Проверка..."); }
inline QString busyConverting() { return text(u8"Расчёт..."); }
inline QString providerAvailable(bool value) {
    return value ? text(u8"Доступен через сервер") : text(u8"Ожидает серверную интеграцию");
}
inline QString providerApiKeyState(bool value) {
    return value ? text(u8"API-ключ нужен") : text(u8"API-ключ не нужен");
}
inline QString providerApiKeyRequiredShort(bool value) {
    return value ? text(u8"Требуется") : text(u8"Не нужен");
}
inline QString providerSelectionHint(bool value) {
    return value
        ? text(u8"Источник можно выбрать для текущей серверной конфигурации.")
        : text(u8"Источник уже есть в каталоге клиента, но пока не подключён на сервере.");
}
inline QString sourcePageTitle() { return text(u8"Главная"); }
inline QString dashboardPageTitle() { return text(u8"Панель управления"); }
inline QString ratesPageTitle() { return text(u8"Курсы"); }
inline QString historyPageTitle() { return text(u8"История"); }
inline QString conversionPageTitle() { return text(u8"Конвертация"); }
inline QString aggregationPageTitle() { return text(u8"Агрегация"); }
inline QString settingsPageTitle() { return text(u8"Настройки"); }

}
