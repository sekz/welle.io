#!/bin/bash
# Simple script to run Thailand DAB+ with Docker

set -e

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}🇹🇭 Thailand DAB+ welle-cli Docker Runner${NC}"
echo "==========================================="

# Check if Docker is running
if ! docker info >/dev/null 2>&1; then
    echo -e "${RED}❌ Docker is not running. Please start Docker first.${NC}"
    exit 1
fi

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  web         Start web interface (default) - http://localhost:8080"
    echo "  build       Build Docker image"
    echo "  dev         Start development shell"
    echo "  clean       Clean up containers and images"
    echo "  help        Show this help"
    echo ""
    echo "Examples:"
    echo "  $0 web      # Start web interface on port 8080"
    echo "  $0 build    # Build the Docker image"
    echo "  $0 dev      # Start development environment"
}

# Default action
ACTION=${1:-web}

case $ACTION in
    build)
        echo -e "${YELLOW}🔨 Building Docker image...${NC}"
        docker build -t welle-cli-simple:latest .
        echo -e "${GREEN}✅ Build completed!${NC}"
        ;;

    web)
        echo -e "${YELLOW}🌐 Starting Thailand DAB+ web interface...${NC}"
        echo "Building image if needed..."
        docker build -t welle-cli-simple:latest . >/dev/null 2>&1 || true

        echo "Starting web server..."
        # Create logs directory
        mkdir -p logs

        # Stop existing container if running
        docker stop welle-cli-thailand >/dev/null 2>&1 || true
        docker rm welle-cli-thailand >/dev/null 2>&1 || true

        # Start new container
        docker run -d \
            --name welle-cli-thailand \
            --restart unless-stopped \
            -p 8080:8080 \
            -v "$(pwd)/logs:/logs" \
            -v "$(pwd)/resources/thailand:/thailand-config:ro" \
            --device /dev/bus/usb:/dev/bus/usb \
            --privileged \
            welle-cli-simple:latest \
            welle-cli -c 12B -w 8080

        echo -e "${GREEN}✅ Thailand DAB+ web interface started!${NC}"
        echo ""
        echo -e "${YELLOW}📡 Web Interface:${NC} http://localhost:8080"
        echo -e "${YELLOW}🔧 Channel:${NC} 12B (Bangkok - 225.648 MHz)"
        echo -e "${YELLOW}📊 Logs:${NC} docker logs welle-cli-thailand"
        echo ""
        echo "To stop: docker stop welle-cli-thailand"
        ;;

    dev)
        echo -e "${YELLOW}🛠️ Starting development environment...${NC}"
        docker build -t welle-cli-simple:latest . >/dev/null 2>&1 || true

        docker run -it --rm \
            --name welle-cli-dev \
            -v "$(pwd):/src" \
            -v "$(pwd)/logs:/logs" \
            --device /dev/bus/usb:/dev/bus/usb \
            --privileged \
            -w /src \
            welle-cli-simple:latest \
            /bin/bash
        ;;

    clean)
        echo -e "${YELLOW}🧹 Cleaning up Docker containers and images...${NC}"
        docker stop welle-cli-thailand >/dev/null 2>&1 || true
        docker rm welle-cli-thailand >/dev/null 2>&1 || true
        docker rmi welle-cli-simple:latest >/dev/null 2>&1 || true
        echo -e "${GREEN}✅ Cleanup completed!${NC}"
        ;;

    help|--help|-h)
        show_usage
        ;;

    *)
        echo -e "${RED}❌ Unknown option: $ACTION${NC}"
        echo ""
        show_usage
        exit 1
        ;;
esac
