#include "app/ClientApplication.hpp"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    currency::client::app::ClientApplication clientApplication(application);
    return clientApplication.run();
}