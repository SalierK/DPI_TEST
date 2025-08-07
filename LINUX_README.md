# GoodbyeDPI для Linux - Руководство пользователя

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](../LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux-orange.svg)](https://www.linux.org/)

## Обзор

Это порт GoodbyeDPI для Linux систем, оптимизированный для Manjaro Linux и других дистрибутивов на базе Arch. Приложение обходит системы глубокой проверки пакетов (DPI) для доступа к заблокированным сайтам без использования VPN.

## Системные требования

- **ОС**: Manjaro Linux, Arch Linux, или другие Linux дистрибутивы
- **Права**: Root привилегии (sudo)
- **Зависимости**: 
  - `iptables`
  - `libnetfilter_queue`
  - `gcc` (для сборки)
  - `make`
  - `pkg-config`

## Быстрая установка

### Автоматическая установка (рекомендуется)

```bash
# Скачайте или клонируйте репозиторий
git clone https://github.com/SalierK/DPI_TEST.git
cd DPI_TEST/src

# Запустите скрипт установки
sudo ./install-linux.sh
```

### Ручная установка

1. **Установите зависимости** (Manjaro/Arch):
```bash
sudo pacman -S iptables libnetfilter_queue base-devel
```

2. **Соберите приложение**:
```bash
cd src
make -f Makefile.linux
```

3. **Установите бинарный файл**:
```bash
sudo cp goodbyedpi-linux /usr/local/bin/
sudo chmod +x /usr/local/bin/goodbyedpi-linux
```

## Использование

### Метод 1: Systemd сервис (рекомендуется)

После установки через `install-linux.sh`:

```bash
# Запустить сервис
sudo systemctl start goodbyedpi

# Включить автозапуск при загрузке системы
sudo systemctl enable goodbyedpi

# Проверить статус
sudo systemctl status goodbyedpi

# Просмотреть логи
sudo journalctl -u goodbyedpi -f
```

### Метод 2: Ручной запуск

```bash
# Настройте iptables правила
sudo iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0

# Запустите GoodbyeDPI
sudo goodbyedpi-linux --auto --verbose

# Для остановки очистите правила iptables
sudo iptables -t mangle -D OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0
```

## Параметры командной строки

```
Опции:
  -p, --port-filter PORT       Фильтровать пакеты по порту назначения
  -r, --redirect-dns IP         Перенаправить DNS запросы на указанный IP
  -s, --set-ttl TTL             Установить пользовательский TTL для пакетов
  -f, --fragment-http           Фрагментировать HTTP пакеты
  -e, --fragment-https          Фрагментировать HTTPS пакеты
  -a, --auto                    Автоматически определить лучший метод
  -w, --whitelist FILE          Загрузить белый список доменов из файла
  -b, --blacklist FILE          Загрузить черный список доменов из файла
  -q, --queue-num NUM           Номер очереди NFQUEUE (по умолчанию: 0)
  -d, --daemon                  Запустить как демон
  -v, --verbose                 Включить подробный вывод
  -h, --help                    Показать справку
      --version                 Показать информацию о версии
```

## Примеры использования

### Основные сценарии

```bash
# Автоматический режим (рекомендуется для большинства случаев)
sudo goodbyedpi-linux --auto --verbose

# Ручная настройка с фрагментацией и TTL
sudo goodbyedpi-linux --fragment-http --fragment-https --set-ttl 64

# С перенаправлением DNS
sudo goodbyedpi-linux --redirect-dns 8.8.8.8 --auto

# Запуск как демон
sudo goodbyedpi-linux --auto --daemon

# Фильтрация только определенных портов
sudo goodbyedpi-linux --port-filter 443 --fragment-https
```

### Настройка для конкретных провайдеров

**Для большинства провайдеров**:
```bash
sudo goodbyedpi-linux --auto --verbose
```

**Для провайдеров с агрессивной фильтрацией**:
```bash
sudo goodbyedpi-linux --fragment-http --fragment-https --set-ttl 64 --redirect-dns 1.1.1.1
```

## Настройка iptables

GoodbyeDPI работает с netfilter/iptables для перехвата пакетов. Автоматически устанавливаются следующие правила:

```bash
# Основное правило для HTTP/HTTPS трафика
iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0

# Дополнительные правила для других портов (опционально)
iptables -t mangle -A OUTPUT -p tcp --dport 8080 -j NFQUEUE --queue-num 0
```

### Ручная настройка iptables

```bash
# Добавить правило
sudo iptables -t mangle -A OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0

# Удалить правило
sudo iptables -t mangle -D OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0

# Просмотреть все правила
sudo iptables -t mangle -L OUTPUT -v --line-numbers
```

## Устранение неполадок

### Проблема: "failed to create netfilter queue handle"

**Решение**: Убедитесь, что правила iptables настроены правильно и программа запущена с root правами.

```bash
# Проверьте правила iptables
sudo iptables -t mangle -L OUTPUT

# Проверьте, что модуль ядра загружен
sudo modprobe nfnetlink_queue
```

### Проблема: Нет доступа к заблокированным сайтам

**Решения**:
1. Попробуйте разные параметры:
```bash
sudo goodbyedpi-linux --fragment-http --fragment-https --set-ttl 64
```

2. Используйте другой DNS:
```bash
sudo goodbyedpi-linux --auto --redirect-dns 8.8.8.8
```

3. Попробуйте другой номер очереди:
```bash
sudo goodbyedpi-linux --auto --queue-num 1
```

### Проблема: Высокая нагрузка на CPU

**Решения**:
- Используйте фильтрацию по портам: `--port-filter 443`
- Ограничьте трафик с помощью более специфичных iptables правил

### Проблема: Приложение не запускается

**Проверки**:
```bash
# Проверьте зависимости
pkg-config --exists libnetfilter_queue && echo "OK" || echo "Отсутствует libnetfilter_queue"

# Проверьте права
ls -l /usr/local/bin/goodbyedpi-linux

# Проверьте логи системы
sudo journalctl -u goodbyedpi -n 50
```

## Удаление

### Автоматическое удаление

```bash
cd DPI_TEST/src
sudo ./uninstall-linux.sh
```

### Ручное удаление

```bash
# Остановить сервис
sudo systemctl stop goodbyedpi
sudo systemctl disable goodbyedpi

# Удалить файлы
sudo rm /usr/local/bin/goodbyedpi-linux
sudo rm /etc/systemd/system/goodbyedpi.service
sudo systemctl daemon-reload

# Очистить iptables правила
sudo iptables -t mangle -D OUTPUT -p tcp --dport 80,443 -j NFQUEUE --queue-num 0
```

## Безопасность

- GoodbyeDPI требует root привилегии для работы с netfilter
- Рекомендуется запускать через systemd сервис, а не напрямую
- Регулярно обновляйте систему и проверяйте логи на подозрительную активность

## Производительность

### Оптимизация для слабых систем

```bash
# Минимальная нагрузка
sudo goodbyedpi-linux --port-filter 443 --fragment-https

# Без подробного вывода
sudo goodbyedpi-linux --auto
```

### Мониторинг производительности

```bash
# Просмотр использования ресурсов
htop -p $(pgrep goodbyedpi-linux)

# Статистика сети
ss -tuln | grep :80
ss -tuln | grep :443
```

## Часто задаваемые вопросы

**Q: Работает ли это с VPN?**
A: Да, но обычно VPN не нужен при использовании GoodbyeDPI.

**Q: Влияет ли на скорость интернета?**
A: Минимальное влияние при правильной настройке. Может быть небольшая задержка из-за обработки пакетов.

**Q: Поддерживаются ли другие дистрибутивы Linux?**
A: Да, но может потребоваться адаптация команд установки пакетов.

**Q: Можно ли использовать без root прав?**
A: Нет, для работы с netfilter/iptables требуются root привилегии.

## Поддержка

- Создайте issue в репозитории GitHub
- Включите в отчет: версию ОС, логи приложения, используемые параметры
- Для диагностики используйте: `sudo goodbyedpi-linux --verbose`

## Лицензия

Этот проект распространяется под лицензией Apache 2.0. См. файл [LICENSE](../LICENSE).

## Разработка

Для участия в разработке:

```bash
git clone https://github.com/SalierK/DPI_TEST.git
cd DPI_TEST/src
make -f Makefile.linux debug
```

Основные файлы:
- `goodbyedpi-linux.c` - основная логика приложения
- `linux_packet.c` - обработка пакетов через netfilter
- `Makefile.linux` - сборка для Linux
- `install-linux.sh` - скрипт установки