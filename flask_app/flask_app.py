#!/usr/bin/env python

import flask

id = 20160564

# Create the application.
APP = flask.Flask(__name__)


@APP.route('/')
def index():
    """ Home 
    """
    return flask.redirect('/<response>')

@APP.route('/update/<ms>')
def update(ms):
    global id
    id = ms
    return ('', 204)


@APP.route('/<response>')
def hello(response):
    """ Displays the page.
    """
    return flask.render_template('page.html', mssv = id, temperature = 37)


if __name__ == '__main__':
    APP.debug=True
    APP.run()