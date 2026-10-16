FROM python:3.12-slim
WORKDIR /app
RUN pip install --no-cache-dir PyYAML
COPY trap.py .
ENV PYTHONDONTWRITEBYTECODE=1
ENV HONEYFORGE_PROFILE=/app/profile.yaml
USER 10001:10001
EXPOSE 8080
CMD ["python", "-u", "trap.py"]
