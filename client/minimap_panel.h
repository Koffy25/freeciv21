/*
 * This file is part of Freeciv21.
 * SPDX-FileCopyrightText: 2022 Louis Moureaux
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include "ui_minimap_panel.h"

// qt-client is one true king
#include "widgetdecorations.h"

class QEvent;
class QGridLayout;
class QItemSelection;
class QListWidget;
class QMouseEvent;
class QObject;
class QPaintEvent;
class QPainter;
class QPushButton;
class QPixmap;
class QResizeEvent;

class map_view;

/**
 * The panel at the bottom right of the game screen, holding the minimap and
 * the Turn Done button.
 */
class minimap_panel : public resizable_widget {
  Q_OBJECT
public:
  explicit minimap_panel(map_view *map, QWidget *parent = nullptr);

  /// Destructor.
  virtual ~minimap_panel() = default;

  void update_menu() override {}

  void set_minimap_visible(bool visible);

  /// Retrieves the minimap widget.
  auto minimap() { return ui.minimap; }

  /// Retrieves the Turn Done button.
  auto turn_done() { return ui.turn_done; }

private:
  Ui::minimap_panel ui;
};

void update_timeout_label();
