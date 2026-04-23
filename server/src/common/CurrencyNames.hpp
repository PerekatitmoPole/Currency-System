#pragma once

#include <string>
#include <unordered_map>

namespace currency::common {

inline std::string russianCurrencyName(const std::string& code, const std::string& fallback = {}) {
    static const std::unordered_map<std::string, std::string> names{
        {"AUD", "Австралийский доллар"},
        {"BGN", "Болгарский лев"},
        {"BRL", "Бразильский реал"},
        {"CAD", "Канадский доллар"},
        {"CHF", "Швейцарский франк"},
        {"CNY", "Китайский юань"},
        {"CZK", "Чешская крона"},
        {"DKK", "Датская крона"},
        {"EUR", "Евро"},
        {"GBP", "Фунт стерлингов"},
        {"HKD", "Гонконгский доллар"},
        {"HUF", "Венгерский форинт"},
        {"IDR", "Индонезийская рупия"},
        {"ILS", "Новый израильский шекель"},
        {"INR", "Индийская рупия"},
        {"ISK", "Исландская крона"},
        {"JPY", "Японская иена"},
        {"KRW", "Южнокорейская вона"},
        {"MXN", "Мексиканское песо"},
        {"MYR", "Малайзийский ринггит"},
        {"NOK", "Норвежская крона"},
        {"NZD", "Новозеландский доллар"},
        {"PHP", "Филиппинское песо"},
        {"PLN", "Польский злотый"},
        {"RON", "Румынский лей"},
        {"RUB", "Российский рубль"},
        {"SEK", "Шведская крона"},
        {"SGD", "Сингапурский доллар"},
        {"THB", "Таиландский бат"},
        {"TRY", "Турецкая лира"},
        {"USD", "Доллар США"},
        {"ZAR", "Южноафриканский рэнд"},
    };

    const auto iterator = names.find(code);
    if (iterator != names.end()) {
        return iterator->second;
    }

    return fallback.empty() ? code : fallback;
}

}
