from flask import Flask
from dotenv import load_dotenv
from .config import Config

load_dotenv()

def create_app():
    app = Flask(__name__)
    app.config.from_object(Config)

    # Register Blueprint
    from .routes import api
    app.register_blueprint(api)

    return app